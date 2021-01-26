/******************************************************************************
Copyright (c) 2021, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/
#include <lisp/util/xmalloc.h>
#include "vm.h"
#include "gc_iterator.h"
#include "gc_stat.h"
#include "error.h"
#include "cell.h"
#include "cell_iterator.h"
#include "tid.h"
#include "cons.h"
#include "complex_object.h"
#include "array.h"

/*****************************************************************************
 Helper functions
 ****************************************************************************/
#define DEFAULT_PAGE_SIZE 1024

static int lisp_erase_list(lisp_gc_collectible_list_t * lst)
{
  lisp_dl_item_t * item;
  while(!lisp_dl_list_empty(&lst->objects))
  {
    item = lst->objects.first;
    lisp_dl_list_remove_first(&lst->objects);
    FREE(item);
  }
  return LISP_OK;
}

int lisp_init_vm(lisp_vm_t * vm)
{
  int ret = lisp_load_static_types();
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_init_color_map(&vm->cons_color_map);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_init_color_map(&vm->object_color_map);
  if(ret != LISP_OK)
  {
    return ret;
  }

  /* cons pages */
  vm->cons_pages = NULL;
  lisp_init_dl_list(&vm->recycled_conses);
  lisp_init_dl_list(&vm->disposed_objects);
  lisp_init_dl_list(&vm->disposed_conses);

  vm->num_cons_pages = 0;
  vm->cons_pos = DEFAULT_PAGE_SIZE;
  vm->cons_page_size = DEFAULT_PAGE_SIZE;

  /* gc cycles */
  vm->num_cycles = 0;
  vm->num_steps = 1;
  return LISP_OK;
}

int lisp_free_vm(lisp_vm_t * vm)
{
  int ret;
  size_t i;
  ret = lisp_vm_gc_full_cycle(vm);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_vm_recycle_all(vm);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_free_color_map(&vm->cons_color_map);
  if(ret != LISP_OK)
  {
    return ret;
  }
  for(i = 0; i < vm->num_cons_pages; i++)
  {
    FREE(vm->cons_pages[i]);
  }
  if(vm->cons_pages)
  {
    FREE(vm->cons_pages);
  }
  ret = lisp_erase_list(vm->object_color_map.white_root);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(vm->object_color_map.grey_root);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(vm->object_color_map.white_root);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(vm->object_color_map.white);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(vm->object_color_map.grey);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(vm->object_color_map.white);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_free_color_map(&vm->object_color_map);
  return ret;
}

/*****************************************************************************
 lisp_cons_t memory management functions
 ****************************************************************************/
int lisp_vm_gc_set_cons_page_size(lisp_vm_t * vm, size_t page_size)
{
  if(vm->num_cons_pages)
  {
    return LISP_INVALID;
  }
  else
  {
    vm->cons_pos = page_size;
    vm->cons_page_size = page_size;
    return LISP_OK;
  }
}

size_t lisp_vm_gc_set_steps(lisp_vm_t * vm, size_t n)
{
  size_t ret = vm->num_steps;
  vm->num_steps = n;
  return ret;
}

/*****************************************************************************
 cast functions
 ****************************************************************************/
inline static lisp_dl_item_t * _lisp_cons_as_dl_item(const lisp_cons_t * cons)
{
  return (lisp_dl_item_t*) (((char*)cons) - sizeof(lisp_dl_item_t));
}

inline static lisp_cons_t * _lisp_dl_as_cons(const lisp_dl_item_t * item)
{
  return ((lisp_cons_t *)(item + 1));
}

inline static lisp_complex_object_t * _lisp_dl_as_complex_object(const lisp_dl_item_t * item)
{
  return ((lisp_complex_object_t*)(item + 1));
}

/****************************************************************************
 lisp_vm_t allocator garbage collector operation
 ****************************************************************************/
void * lisp_vm_alloc_root_complex_object(lisp_vm_t * vm,
                                         lisp_type_id_t tid,
                                         size_t size)
{
  lisp_dl_item_t * item;
  lisp_complex_object_t * obj;
  lisp_gc_collectible_list_t * list;
  assert(LISP_IS_STORAGE_COMPLEX_TID(tid));
  list = vm->object_color_map.white_root;
  item = (lisp_dl_item_t*) MALLOC(sizeof(lisp_dl_item_t) +
                                  sizeof(lisp_complex_object_t) +
                                  size);
  lisp_dl_list_append(&list->objects, item);
  obj = (lisp_complex_object_t *)(((char*)item) + sizeof(lisp_dl_item_t));
  obj->gc_list = list;
  obj->ref_count = 1u;
  obj->type_id = tid;
  return (((char*) item) +
          sizeof(lisp_dl_item_t) +
          sizeof(lisp_complex_object_t));
}

inline static void _move_cons_list(lisp_gc_collectible_list_t * target,
                                   lisp_gc_collectible_list_t * source)
{
  lisp_dl_item_t * item = source->objects.first;
  while(item != NULL)
  {
    _lisp_dl_as_cons(item)->gc_list = target;
    item = item->next;
  }
  lisp_dl_list_move_list(&target->objects, &source->objects);
}

inline static void _move_conses(lisp_gc_collectible_list_t * target,
                                lisp_dl_list_t * recycled_conses,
                                lisp_dl_item_t * item,
                                hash_table_t * ht)
{
  lisp_cell_t cell;
  lisp_dl_item_t * next;
  while(item != NULL)
  {
    next = item->next;
    cell.type_id = LISP_TID_CONS;
    cell.data.obj = _lisp_dl_as_cons(item);
    if(hash_table_find(ht, &cell) == NULL)
    {
      lisp_dl_list_append(recycled_conses, item);
    }
    else
    {
      lisp_dl_list_append(&target->objects, item);
    }
    item = next;
  }
}

inline static void _move_object_list(lisp_gc_collectible_list_t * target,
                                     lisp_gc_collectible_list_t * source)
{
  lisp_dl_item_t * item = source->objects.first;
  while(item != NULL)
  {
    _lisp_dl_as_complex_object(item)->gc_list = target;
    item = item->next;
  }
  lisp_dl_list_move_list(&target->objects, &source->objects);
}

inline static void _move_objects(lisp_gc_collectible_list_t * target,
                                 lisp_dl_list_t * recycled_conses,
                                 lisp_dl_item_t * item,
                                 hash_table_t * ht)
{
  lisp_cell_t cell;
  lisp_dl_item_t * next;
  while(item != NULL)
  {
    next = item->next;
    cell.type_id = LISP_TID_CONS;
    cell.data.obj = _lisp_dl_as_cons(item);
    if(hash_table_find(ht, &cell) == NULL)
    {
      lisp_dl_list_append(recycled_conses, item);
    }
    else
    {
      lisp_dl_list_append(&target->objects, item);
    }
    item = next;
  }
}

int lisp_vm_gc_full_cycle(lisp_vm_t * vm)
{
  lisp_gc_reachable_iterator_t ritr;
  lisp_dl_item_t * item;
  lisp_init_gc_reachable_iterator(&ritr);
  for(lisp_gc_reachable_first(vm, &ritr);
      lisp_gc_reachable_iterator_is_valid(&ritr);
      lisp_gc_reachable_next(vm, &ritr))
  {
  }

  /* conses */
  _move_cons_list(vm->cons_color_map.white_root,
                  vm->cons_color_map.grey_root);
  _move_cons_list(vm->cons_color_map.white_root,
                  vm->cons_color_map.black_root);

  item = vm->cons_color_map.white->objects.first;
  vm->cons_color_map.white->objects.first = NULL;
  vm->cons_color_map.white->objects.last = NULL;
  _move_conses(vm->cons_color_map.white,
               &vm->disposed_conses,
               item,
               &ritr.root);
  _move_conses(vm->cons_color_map.white,
               &vm->disposed_conses,
               vm->cons_color_map.grey->objects.first,
               &ritr.root);
  vm->cons_color_map.grey->objects.first = NULL;
  vm->cons_color_map.grey->objects.last = NULL;
  _move_conses(vm->cons_color_map.white,
               &vm->disposed_conses,
               vm->cons_color_map.black->objects.first,
               &ritr.root);
  vm->cons_color_map.black->objects.first = NULL;
  vm->cons_color_map.black->objects.last = NULL;

  /* objects */
  _move_object_list(vm->object_color_map.white_root,
                    vm->object_color_map.grey_root);
  _move_object_list(vm->object_color_map.white_root,
                    vm->object_color_map.black_root);
  item = vm->object_color_map.white->objects.first;
  vm->object_color_map.white->objects.first = NULL;
  vm->object_color_map.white->objects.last = NULL;
  _move_objects(vm->object_color_map.white,
                &vm->disposed_objects,
                item,
                &ritr.root);
  _move_objects(vm->object_color_map.white,
                &vm->disposed_objects,
                vm->object_color_map.grey->objects.first,
                &ritr.root);
  vm->object_color_map.grey->objects.first = NULL;
  vm->object_color_map.grey->objects.last = NULL;
  _move_objects(vm->object_color_map.white,
                &vm->disposed_objects,
                vm->object_color_map.black->objects.first,
                &ritr.root);
  vm->object_color_map.black->objects.first = NULL;
  vm->object_color_map.black->objects.last = NULL;

  lisp_free_gc_reachable_iterator(&ritr);
  vm->num_cycles++;
  return LISP_OK;
}

int lisp_vm_recycle_all_conses(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  for(item = vm->disposed_conses.first;
      item != NULL;
      item = item->next)
  {
    /* @todo unset car & cdr */
    /* _unset_cell(&_lisp_dl_as_cons(item)->car); */
    /* _unset_cell(&_lisp_dl_as_cons(item)->cdr); */
  }
  lisp_dl_list_move_list(&vm->recycled_conses,
                         &vm->disposed_conses);
  return LISP_OK;
}

int lisp_vm_recycle_all_objects(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  lisp_dl_item_t * next;
  lisp_cell_iterator_t citr;
  lisp_cell_t cell;
  lisp_type_t * type;
  lisp_complex_object_t * obj;
  item = vm->disposed_objects.first;
  while(item)
  {
    obj = _lisp_dl_as_complex_object(item);
    assert(obj->ref_count == 0);
    assert(LISP_IS_STORAGE_COMPLEX_TID(obj->type_id));
    assert(obj->type_id < LISP_NUM_TYPES);
    cell.type_id = obj->type_id;
    cell.data.obj = obj + 1;
    for(lisp_first_child(&cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      /* @todo unset car & cdr */
      /* _unset_cell(&citr.cell); */
    }
    next = item->next;
    type = &lisp_static_types[_lisp_dl_as_complex_object(item)->type_id];
    if(type->lisp_destructor_ptr)
    {
      type->lisp_destructor_ptr(cell.data.obj);
    }
    FREE(item);
    item = next;
  }
  return LISP_OK;
}

int lisp_vm_recycle_all(lisp_vm_t * vm)
{
  int ret = lisp_vm_recycle_all_conses(vm);
  if(ret != LISP_OK)
  {
    return ret;
  }
  return lisp_vm_recycle_all_objects(vm);
}

static inline short int _lisp_vm_gc_cons_step(lisp_vm_t * vm)
{
  /* @todo implement function */
  return 0;
}

static inline short int _lisp_vm_gc_object_step(lisp_vm_t * vm)
{
  /* @todo implement function */
  return 0;
}

void lisp_vm_gc_step(lisp_vm_t * vm)
{
  short int swapable = 1;
  swapable &= _lisp_vm_gc_cons_step(vm);
  swapable &= _lisp_vm_gc_object_step(vm);
  if(swapable)
  {
    vm->num_cycles++;
    /*
      @todo implement swap
    */
  }
}

short int lisp_vm_gc_cons_step(lisp_vm_t * vm)
{
  return _lisp_vm_gc_cons_step(vm);
}

short int lisp_vm_gc_object_step(lisp_vm_t * vm)
{
  return _lisp_vm_gc_object_step(vm);
}

/*****************************************************************************
 constructors
 ****************************************************************************/
static lisp_cons_t * _lisp_gc_alloc_cons(lisp_vm_t * vm,
                                         lisp_gc_collectible_list_t * list,
                                         size_t ref_count)
{
  lisp_dl_item_t * item;
  if(!lisp_dl_list_empty(&vm->recycled_conses))
  {
    item = vm->recycled_conses.first;
    lisp_dl_list_remove_first(&vm->recycled_conses);
  }
  else
  {
    if(vm->cons_pos >= vm->cons_page_size)
    {
      vm->current_cons_page = MALLOC((sizeof(lisp_dl_item_t) +
                                      sizeof(lisp_cons_t)) * vm->cons_page_size);
      vm->cons_pages = REALLOC(vm->cons_pages,
                               sizeof(void*) * (vm->num_cons_pages + 1));
      if(!vm->cons_pages)
      {
        return NULL;
      }
      vm->cons_pages[vm->num_cons_pages++] = vm->current_cons_page;
      vm->cons_pos = 0;
    }
    item = (lisp_dl_item_t*)(
                             (char*) vm->current_cons_page +
                             ((
                               sizeof(lisp_dl_item_t) +
                               sizeof(lisp_cons_t)) *
                              vm->cons_pos++));
  }
  lisp_dl_list_append(&list->objects, item);
  lisp_cons_t * ret = (lisp_cons_t*)((
                                      (char*) item) +
                                     sizeof(lisp_dl_item_t));
  ret->ref_count = ref_count;
  ret->gc_list = list;
  ret->car.type_id = LISP_TID_NIL;
  ret->cdr.type_id = LISP_TID_NIL;
  return ret;
}

static int _lisp_make_cons_cell(lisp_vm_t * vm,
                                lisp_cell_t * target,
                                const lisp_cell_t * source)
{
  if(source)
  {
    switch(LISP_STORAGE_ID(source->type_id))
    {
    case LISP_STORAGE_ATOM:
      target->data = source->data;
    case LISP_STORAGE_NULL:
      target->type_id = source->type_id;
      return LISP_OK;
      break;
    case LISP_STORAGE_COW_OBJECT:
      break;
    case LISP_STORAGE_OBJECT:
      break;
    case LISP_STORAGE_CONS:
      /* ensure that child is not white. */
      lisp_cons_grey((lisp_cons_t*)source->data.obj);
      target->type_id = source->type_id;
      target->data = source->data;
      return LISP_OK;
    case LISP_STORAGE_COMPLEX:
      break;
    }
    /*@todo implement other types */
    return LISP_NOT_IMPLEMENTED;
  }
  else
  {
    target->type_id = LISP_TID_NIL;
  }
  return LISP_OK;
}

int lisp_make_cons(lisp_vm_t * vm,
                   lisp_cell_t * cell,
                   const lisp_cell_t * car,
                   const lisp_cell_t * cdr)
{
  int ret;
  cell->type_id = LISP_TID_CONS;
  cell->data.obj = _lisp_gc_alloc_cons(vm,
                                       vm->cons_color_map.white_root,
                                       1u);

  if(!cell->data.obj)
  {
    return LISP_BAD_ALLOC;
  }
  ret = _lisp_make_cons_cell(vm,
                             &((lisp_cons_t*)cell->data.obj)->car,
                             car);
  if(!ret)
  {
    return _lisp_make_cons_cell(vm,
                                &((lisp_cons_t*)cell->data.obj)->cdr,
                                cdr);
  }
  else
  {
    return ret;
  }
}

/****************************************************************************
 lisp_vm_t consistency checks, dump and statistics
 ****************************************************************************/
static void _lisp_gc_dump_humamn(FILE * fp, lisp_vm_t * vm)
{
  lisp_gc_stat_t stat;
  lisp_gc_iterator_t itr;
  size_t num_conses = 0;
  lisp_vm_gc_get_stats(vm, &stat);
  for(lisp_gc_first(vm, &itr);
      lisp_gc_iterator_is_valid(&itr);
      lisp_gc_next(vm, &itr))
  {
    if(LISP_IS_STORAGE_CONS_TID(itr.cell.type_id))
    {
      num_conses++;
    }
    fprintf(fp, "OBJ %p\n", itr.cell.data.obj);
  }
  fprintf(fp, "\n");
  lisp_gc_stat_print(fp, &stat);
}

int lisp_vm_gc_check(lisp_vm_t * vm)
{
  /* @todo more checks */
  lisp_gc_stat_t stat;
  lisp_gc_iterator_t itr;
  size_t num_conses = 0;
  size_t num_allocated_conses = 0;
  lisp_vm_gc_get_stats(vm, &stat);
  if(stat.error_black_has_white_child)
  {
    return LISP_INVALID;
  }
  for(lisp_gc_first(vm, &itr);
      lisp_gc_iterator_is_valid(&itr);
      lisp_gc_next(vm, &itr))
  {
    if(LISP_IS_STORAGE_CONS_TID(itr.cell.type_id))
    {
      num_conses++;
    }
  }
  if(vm->num_cons_pages)
  {
    num_allocated_conses = (vm->num_cons_pages - 1) * vm->cons_page_size + vm->cons_pos;
  }
  if(num_allocated_conses !=
     (num_conses +
      lisp_dl_list_size(&vm->recycled_conses) +
      lisp_dl_list_size(&vm->disposed_conses)))
  {
    return LISP_INVALID;
  }
  return LISP_OK;
}

void lisp_vm_gc_dump(FILE * fp, lisp_vm_t * vm, int mode)
{
  switch(mode)
  {
  case LISP_GC_DUMP_HUMAN:
    _lisp_gc_dump_humamn(fp, vm);
    break;
  }
}

void lisp_vm_gc_get_stats(lisp_vm_t * vm,
                          struct lisp_gc_stat_t * stat)
{
  lisp_gc_reachable_iterator_t ritr;
  lisp_cell_iterator_t citr;
  lisp_gc_color_t parent_color;
  lisp_init_gc_reachable_iterator(&ritr);
  lisp_init_gc_stat(stat);
  stat->num_cycles = vm->num_cycles;
  stat->num_cons_pages = vm->num_cons_pages;
  stat->num_root =
    lisp_dl_list_size(&vm->cons_color_map.white_root->objects) +
    lisp_dl_list_size(&vm->cons_color_map.grey_root->objects) +
    lisp_dl_list_size(&vm->cons_color_map.black_root->objects) +
    lisp_dl_list_size(&vm->object_color_map.white_root->objects) +
    lisp_dl_list_size(&vm->object_color_map.grey_root->objects) +
    lisp_dl_list_size(&vm->object_color_map.black_root->objects);
  stat->num_allocated =
    stat->num_root +
    lisp_dl_list_size(&vm->cons_color_map.white->objects) +
    lisp_dl_list_size(&vm->cons_color_map.grey->objects) +
    lisp_dl_list_size(&vm->cons_color_map.black->objects) +
    lisp_dl_list_size(&vm->object_color_map.white->objects) +
    lisp_dl_list_size(&vm->object_color_map.grey->objects) +
    lisp_dl_list_size(&vm->object_color_map.black->objects);
  stat->num_reachable = 0;
  for(lisp_gc_reachable_first(vm, &ritr);
      lisp_gc_reachable_iterator_is_valid(&ritr);
      lisp_gc_reachable_next(vm, &ritr))
  {
    stat->num_reachable++;
    parent_color = lisp_get_cell_color(&ritr.cell);
    for(lisp_first_child(&ritr.cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      if(LISP_IS_STORAGE_COMPLEX_OR_CONS(citr.child->type_id))
      {
        if(parent_color == LISP_GC_BLACK &&
           lisp_get_cell_color(citr.child) == LISP_GC_WHITE)
        {
          stat->error_black_has_white_child = 1;
        }
        stat->num_edges++;
      }
      else
      {
        stat->num_leaves++;
      }
    }
  }
  stat->num_bulk = stat->num_reachable - stat->num_root;
  stat->num_recycled = lisp_dl_list_size(&vm->recycled_conses);
  stat->num_void = (vm->cons_page_size - vm->cons_pos);
  stat->num_disposed =
    lisp_dl_list_size(&vm->disposed_conses) +
    lisp_dl_list_size(&vm->disposed_objects);
  lisp_free_gc_reachable_iterator(&ritr);
}
