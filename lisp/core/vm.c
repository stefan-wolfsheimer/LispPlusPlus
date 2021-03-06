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

#define DEFAULT_PAGE_SIZE 1024

/*****************************************************************************
 Helper functions
 ****************************************************************************/
inline static lisp_dl_item_t * _lisp_cons_as_dl_item(const lisp_cons_t * cons)
{
  return (lisp_dl_item_t*) (((lisp_dl_item_t*)cons) - 1);
}

inline static lisp_dl_item_t *
_lisp_complex_object_as_dl_item(const lisp_complex_object_t * obj)
{
  return (lisp_dl_item_t*) (((lisp_dl_item_t*)obj) - 1);
}

inline static lisp_cons_t * _lisp_dl_as_cons(const lisp_dl_item_t * item)
{
  return ((lisp_cons_t *)(item + 1));
}

inline static lisp_complex_object_t * _lisp_dl_as_complex_object(const lisp_dl_item_t * item)
{
  return ((lisp_complex_object_t*)(item + 1));
}

static inline int lisp_erase_list(lisp_gc_collectible_list_t * lst)
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

/*****************************************************************************
 constructor
 ****************************************************************************/
int lisp_init_vm(lisp_vm_t * vm)
{
  int ret = lisp_load_static_types();
  if(ret != LISP_OK)
  {
    return ret;
  }
  if((ret = lisp_init_collectible_lists(vm->cons_lists, vm)) != LISP_OK)
  {
    return ret;
  }
  if((ret = lisp_init_collectible_lists(vm->object_lists, vm)) != LISP_OK)
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
  for(i = 0; i < vm->num_cons_pages; i++)
  {
    FREE(vm->cons_pages[i]);
  }
  if(vm->cons_pages)
  {
    FREE(vm->cons_pages);
  }
  for(i = 0; i < LISP_GC_NUM_CLASSES; i++)
  {
    if((ret = lisp_erase_list(vm->object_lists[i]) != LISP_OK))
    {
      return ret;
    }
  }
  if((ret = lisp_free_collectible_lists(vm->cons_lists)) != LISP_OK)
  {
    return ret;
  }
  if((ret = lisp_free_collectible_lists(vm->object_lists)) != LISP_OK)
  {
    return ret;
  }
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

/****************************************************************************
 lisp_vm_t allocator garbage collector operation
 ****************************************************************************/
inline static
void * _lisp_vm_alloc_complex_object(lisp_vm_t * vm,
                                     lisp_gc_collectible_list_t * list,
                                     lisp_type_id_t tid,
                                     size_t size,
                                     size_t ref_count)
{
  lisp_dl_item_t * item;
  lisp_complex_object_t * obj;
  assert(LISP_STORAGE_ID(tid) == LISP_STORAGE_COMPLEX);
  item = (lisp_dl_item_t*) MALLOC(sizeof(lisp_dl_item_t) +
                                  sizeof(lisp_complex_object_t) +
                                  size);
  lisp_dl_list_append(&list->objects, item);
  obj = (lisp_complex_object_t *)(((char*)item) + sizeof(lisp_dl_item_t));
  obj->gc_list = list;
  obj->ref_count = ref_count;
  obj->type_id = tid;
  return (((char*) item) +
          sizeof(lisp_dl_item_t) +
          sizeof(lisp_complex_object_t));

}


void * lisp_vm_alloc_root_complex_object(lisp_vm_t * vm,
                                         lisp_type_id_t tid,
                                         size_t size)
{
  return _lisp_vm_alloc_complex_object(vm,
                                       vm->object_lists[LISP_GC_WHITE_ROOT],
                                       tid,
                                       size,
                                       1u);
}

void * lisp_vm_alloc_temp_complex_object(lisp_vm_t * vm,
                                         lisp_type_id_t tid,
                                         size_t size)
{
  return _lisp_vm_alloc_complex_object(vm,
                                       vm->object_lists[LISP_GC_GREY],
                                       tid,
                                       size,
                                       0u);
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
  _move_cons_list(vm->cons_lists[LISP_GC_WHITE_ROOT],
                  vm->cons_lists[LISP_GC_GREY_ROOT]);
  _move_cons_list(vm->cons_lists[LISP_GC_WHITE_ROOT],
                  vm->cons_lists[LISP_GC_BLACK_ROOT]);

  item = vm->cons_lists[LISP_GC_WHITE]->objects.first;
  vm->cons_lists[LISP_GC_WHITE]->objects.first = NULL;
  vm->cons_lists[LISP_GC_WHITE]->objects.last = NULL;
  _move_conses(vm->cons_lists[LISP_GC_WHITE],
               &vm->disposed_conses,
               item,
               &ritr.root);
  _move_conses(vm->cons_lists[LISP_GC_WHITE],
               &vm->disposed_conses,
               vm->cons_lists[LISP_GC_GREY]->objects.first,
               &ritr.root);
  vm->cons_lists[LISP_GC_GREY]->objects.first = NULL;
  vm->cons_lists[LISP_GC_GREY]->objects.last = NULL;
  _move_conses(vm->cons_lists[LISP_GC_WHITE],
               &vm->disposed_conses,
               vm->cons_lists[LISP_GC_BLACK]->objects.first,
               &ritr.root);
  vm->cons_lists[LISP_GC_BLACK]->objects.first = NULL;
  vm->cons_lists[LISP_GC_BLACK]->objects.last = NULL;

  /* objects */
  _move_object_list(vm->object_lists[LISP_GC_WHITE_ROOT],
                    vm->object_lists[LISP_GC_GREY_ROOT]);
  _move_object_list(vm->object_lists[LISP_GC_WHITE_ROOT],
                    vm->object_lists[LISP_GC_BLACK_ROOT]);
  item = vm->object_lists[LISP_GC_WHITE]->objects.first;
  vm->object_lists[LISP_GC_WHITE]->objects.first = NULL;
  vm->object_lists[LISP_GC_WHITE]->objects.last = NULL;
  _move_objects(vm->object_lists[LISP_GC_WHITE],
                &vm->disposed_objects,
                item,
                &ritr.root);
  _move_objects(vm->object_lists[LISP_GC_WHITE],
                &vm->disposed_objects,
                vm->object_lists[LISP_GC_GREY]->objects.first,
                &ritr.root);
  vm->object_lists[LISP_GC_GREY]->objects.first = NULL;
  vm->object_lists[LISP_GC_GREY]->objects.last = NULL;
  _move_objects(vm->object_lists[LISP_GC_WHITE],
                &vm->disposed_objects,
                vm->object_lists[LISP_GC_BLACK]->objects.first,
                &ritr.root);
  vm->object_lists[LISP_GC_BLACK]->objects.first = NULL;
  vm->object_lists[LISP_GC_BLACK]->objects.last = NULL;

  lisp_free_gc_reachable_iterator(&ritr);
  vm->num_cycles++;
  return LISP_OK;
}

int lisp_vm_recycle_next_cons(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  int ret = LISP_NO_CHANGE;
  if(vm->disposed_conses.first)
  {
    item = vm->disposed_conses.first;
    ret = lisp_cons_unset_car_cdr(_lisp_dl_as_cons(item));
    lisp_dl_list_remove_first(&vm->disposed_conses);
    lisp_dl_list_append(&vm->recycled_conses, item);
  }
  return ret;
}

int lisp_vm_recycle_all_conses(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  int ret = LISP_OK;
  int r;
  for(item = vm->disposed_conses.first;
      item != NULL;
      item = item->next)
  {
    r = lisp_cons_unset_car_cdr(_lisp_dl_as_cons(item));
    if(r != LISP_OK)
    {
      ret = r;
    }
  }
  lisp_dl_list_move_list(&vm->recycled_conses,
                         &vm->disposed_conses);
  return ret;
}

int lisp_vm_recycle_all_objects(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  lisp_dl_item_t * next;
  lisp_cell_iterator_t citr;
  lisp_cell_t cell;
  lisp_type_t * type;
  lisp_complex_object_t * obj;
  int ret, r;
  ret = LISP_OK;
  item = vm->disposed_objects.first;
  while(item)
  {
    obj = _lisp_dl_as_complex_object(item);
    assert(obj->ref_count == 0);
    assert(LISP_STORAGE_ID(obj->type_id) == LISP_STORAGE_COMPLEX);
    assert(obj->type_id < LISP_NUM_TYPES);
    cell.type_id = obj->type_id;
    cell.data.obj = obj + 1;
    for(lisp_first_child(&cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      r = lisp_unset_child_cell(citr.child);
      if(r != LISP_OK)
      {
        ret = r;
      }
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
  return ret;
}

int lisp_vm_recycle_next_object(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  int ret,r;
  lisp_complex_object_t * obj;
  lisp_cell_t cell;
  lisp_cell_iterator_t citr;
  lisp_type_t * type;
  if(vm->disposed_objects.first)
  {
    item = vm->disposed_objects.first;
    obj = _lisp_dl_as_complex_object(item);
    lisp_dl_list_remove_first(&vm->disposed_objects);
    ret = LISP_OK;
    assert(obj->ref_count == 0);
    assert(LISP_STORAGE_ID(obj->type_id) == LISP_STORAGE_COMPLEX);
    assert(obj->type_id < LISP_NUM_TYPES);
    cell.type_id = obj->type_id;
    cell.data.obj = obj + 1;

    /*@todo do it stepwise */
    for(lisp_first_child(&cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      r = lisp_unset_child_cell(citr.child);
      if(r != LISP_OK)
      {
        ret = r;
      }
    }
    type = &lisp_static_types[obj->type_id];
    if(type->lisp_destructor_ptr)
    {
      type->lisp_destructor_ptr(obj + 1);
    }
    FREE(item);
    return ret;
  }
  else
  {
    return LISP_NO_CHANGE;
  }
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

static inline void _lisp_grey_cell(lisp_cell_t * cell)
{
  switch(LISP_STORAGE_ID(cell->type_id))
  {
    /* @todo implement storage complex
  case LISP_STORAGE_COMPLEX:
    assert(0);
    break; */
  case LISP_STORAGE_CONS:
    lisp_cons_grey((lisp_cons_t*)cell->data.obj);
    break;
  }
}

static inline short int _lisp_vm_gc_cons_step(lisp_vm_t * vm)
{
  lisp_dl_item_t * item;
  lisp_cons_t * cons;
  if(vm->cons_lists[LISP_GC_WHITE_ROOT]->objects.first)
  {
    item = vm->cons_lists[LISP_GC_WHITE_ROOT]->objects.first;
    cons = _lisp_dl_as_cons(item);
    /* note that in rare cases cons itself can be moved while
       during greying cells */
    _lisp_grey_cell(&cons->car);
    _lisp_grey_cell(&cons->cdr);
    lisp_dl_list_remove(&cons->gc_list->objects, item);
    lisp_dl_list_append(&vm->cons_lists[LISP_GC_BLACK_ROOT]->objects, item);
    _lisp_dl_as_cons(item)->gc_list = vm->cons_lists[LISP_GC_BLACK_ROOT];
    return
      vm->cons_lists[LISP_GC_WHITE_ROOT]->objects.first == NULL &&
      vm->cons_lists[LISP_GC_GREY_ROOT]->objects.first == NULL &&
      vm->cons_lists[LISP_GC_GREY]->objects.first == NULL;
  }
  else if(vm->cons_lists[LISP_GC_GREY_ROOT]->objects.first)
  {
    item = vm->cons_lists[LISP_GC_GREY_ROOT]->objects.first;
    cons = _lisp_dl_as_cons(item);
    _lisp_grey_cell(&cons->car);
    _lisp_grey_cell(&cons->cdr);
    lisp_dl_list_remove(&cons->gc_list->objects, item);
    lisp_dl_list_append(&vm->cons_lists[LISP_GC_BLACK_ROOT]->objects, item);
    _lisp_dl_as_cons(item)->gc_list = vm->cons_lists[LISP_GC_BLACK_ROOT];
    return
      vm->cons_lists[LISP_GC_GREY_ROOT]->objects.first == NULL &&
      vm->cons_lists[LISP_GC_GREY]->objects.first == NULL;
  }
  else if(vm->cons_lists[LISP_GC_GREY]->objects.first)
  {
    item = vm->cons_lists[LISP_GC_GREY]->objects.first;
    cons = _lisp_dl_as_cons(item);
    _lisp_grey_cell(&cons->car);
    _lisp_grey_cell(&cons->cdr);
    lisp_dl_list_remove(&cons->gc_list->objects, item);
    lisp_dl_list_append(&vm->cons_lists[LISP_GC_BLACK]->objects, item);
    _lisp_dl_as_cons(item)->gc_list = vm->cons_lists[LISP_GC_BLACK];
    return
      vm->cons_lists[LISP_GC_GREY]->objects.first == NULL;
  }
  else
  {
    /* nothing left */
    return 1;
  }
}

static inline short int _lisp_vm_gc_object_grey_child(lisp_gc_collectible_list_t * from,
                                                      lisp_gc_collectible_list_t * to)
{
  lisp_dl_item_t * item;
  lisp_complex_object_t * obj;
  lisp_cell_t * cell;
  item = from->objects.first;
  obj = _lisp_dl_as_complex_object(item);
  assert(obj->type_id < LISP_NUM_TYPES);
  assert(lisp_static_types[obj->type_id].grey_next_child_ptr);
  cell = lisp_static_types[obj->type_id].grey_next_child_ptr(obj + 1);
  if(cell)
  {
    _lisp_grey_cell(cell);
    return 0;
  }
  else
  {
    lisp_dl_list_remove_first(&from->objects);
    lisp_dl_list_append(&to->objects, item);
    _lisp_dl_as_complex_object(item)->gc_list = to;
    return 1;
  }

}

static inline short int _lisp_vm_gc_object_step(lisp_vm_t * vm)
{
  /* @todo test coverage */
  if(vm->object_lists[LISP_GC_WHITE_ROOT]->objects.first)
  {
    if(_lisp_vm_gc_object_grey_child(vm->object_lists[LISP_GC_WHITE_ROOT],
                                     vm->object_lists[LISP_GC_BLACK_ROOT]))
    {
      return
        vm->object_lists[LISP_GC_WHITE_ROOT]->objects.first == NULL &&
        vm->object_lists[LISP_GC_GREY_ROOT]->objects.first == NULL &&
        vm->object_lists[LISP_GC_GREY]->objects.first == NULL;
    }
    else
    {
      return 0;
    }
  }
  else if(vm->object_lists[LISP_GC_GREY_ROOT]->objects.first)
  {
    if(_lisp_vm_gc_object_grey_child(vm->object_lists[LISP_GC_GREY_ROOT],
                                     vm->object_lists[LISP_GC_BLACK_ROOT]))
    {
      return
        vm->object_lists[LISP_GC_GREY_ROOT]->objects.first == NULL &&
        vm->object_lists[LISP_GC_GREY]->objects.first == NULL;
    }
    else
    {
      return 0;
    }
  }
  else if(vm->object_lists[LISP_GC_GREY]->objects.first)
  {
    if(_lisp_vm_gc_object_grey_child(vm->object_lists[LISP_GC_GREY],
                                     vm->object_lists[LISP_GC_BLACK]))
    {
      return vm->object_lists[LISP_GC_GREY]->objects.first == NULL;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    /* nothing left */
    return 1;
  }
}

static inline short int _lisp_vm_gc_swappable(lisp_vm_t * vm)
{
  return
    vm->cons_lists[LISP_GC_WHITE_ROOT]->objects.first == NULL &&
    vm->cons_lists[LISP_GC_GREY_ROOT]->objects.first == NULL &&
    vm->cons_lists[LISP_GC_GREY]->objects.first == NULL &&
    vm->object_lists[LISP_GC_WHITE_ROOT]->objects.first == NULL &&
    vm->object_lists[LISP_GC_GREY_ROOT]->objects.first == NULL &&
    vm->object_lists[LISP_GC_GREY]->objects.first == NULL;
}

void _lisp_vm_gc_swap_list(lisp_gc_collectible_list_t ** _target,
                           lisp_gc_collectible_list_t ** _source)
{
  /* @todo remove is_cons arg */
  assert((*_target)->objects.first == NULL);
  assert((*_target)->objects.last == NULL);
  lisp_gc_collectible_list_t * tmp;
  lisp_gc_color_t color;
  short int is_root;

  tmp = *_target;
  *_target = *_source;
  *_source = tmp;

  color = (*_target)->color;
  (*_target)->color = (*_source)->color;
  (*_source)->color = color;

  is_root = (*_target)->is_root;
  (*_target)->is_root = (*_source)->is_root;
  (*_source)->is_root = is_root;
}

void _lisp_vm_gc_swap(lisp_vm_t * vm)
{
  assert(_lisp_vm_gc_swappable(vm));
  vm->num_cycles++;

  /* white objects are not reachable -> dispose them */
  lisp_dl_list_move_list(&vm->disposed_conses,
                         &vm->cons_lists[LISP_GC_WHITE]->objects);
  lisp_dl_list_move_list(&vm->disposed_objects,
                         &vm->object_lists[LISP_GC_WHITE]->objects);

  _lisp_vm_gc_swap_list(&vm->cons_lists[LISP_GC_WHITE_ROOT],
                        &vm->cons_lists[LISP_GC_BLACK_ROOT]);
  _lisp_vm_gc_swap_list(&vm->cons_lists[LISP_GC_WHITE],
                        &vm->cons_lists[LISP_GC_BLACK]);
  lisp_collectible_list_refresh(vm->cons_lists);
  _lisp_vm_gc_swap_list(&vm->object_lists[LISP_GC_WHITE_ROOT],
                        &vm->object_lists[LISP_GC_BLACK_ROOT]);
  _lisp_vm_gc_swap_list(&vm->object_lists[LISP_GC_WHITE],
                        &vm->object_lists[LISP_GC_BLACK]);
  lisp_collectible_list_refresh(vm->object_lists);
}

short int lisp_vm_gc_step(lisp_vm_t * vm)
{
  short int swappable = 1;
  swappable &= _lisp_vm_gc_cons_step(vm);
  swappable &= _lisp_vm_gc_object_step(vm);
  if(swappable)
  {
    _lisp_vm_gc_swap(vm);
    return 1;
  }
  return 0;
}


short int lisp_vm_gc_swappable(lisp_vm_t * vm)
{
  return _lisp_vm_gc_swappable(vm);
}

short int lisp_vm_gc_swap(lisp_vm_t * vm)
{
  if(lisp_vm_gc_swappable(vm))
  {
    _lisp_vm_gc_swap(vm);
    return 1;
  }
  else
  {
    return 0;
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
    if(LISP_STORAGE_ID(itr.cell.type_id) == LISP_STORAGE_CONS)
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
    if(LISP_STORAGE_ID(itr.cell.type_id) == LISP_STORAGE_CONS)
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

static lisp_gc_collectible_list_t * _lisp_dl_as_coll_list_cons(const lisp_dl_item_t * item)
{
  return _lisp_dl_as_cons(item)->gc_list;
}

static lisp_gc_collectible_list_t * _lisp_dl_as_coll_list_object(const lisp_dl_item_t * item)
{
  return _lisp_dl_as_complex_object(item)->gc_list;
}

inline static size_t _lisp_vm_check_object_list(lisp_dl_list_t * lst,
                                                lisp_gc_collectible_list_t * (*list_getter)(const lisp_dl_item_t * item),
                                                int * err)
{
  lisp_dl_item_t * item;
  size_t n;
  lisp_gc_collectible_list_t * clst;
  n = 0;
  item = lst->first;
  if(lst->first)
  {
    if(lst->last == NULL ||
       lst->first->prev != NULL ||
       lst->last->next != NULL)
    {
      *err = 1;
    }
  }
  else
  {
    if(lst->last != NULL)
    {
      *err = 1;
    }
  }
  for(item = lst->first; item != NULL; item = item->next)
  {
    clst = list_getter(item);
    if(&clst->objects != lst)
    {
      *err = 1;
    }
    n++;
  }
  assert(n == lisp_dl_list_size(lst));
  return n;
}

void lisp_vm_gc_get_stats(lisp_vm_t * vm,
                          struct lisp_gc_stat_t * stat)
{
  lisp_gc_reachable_iterator_t ritr;
  lisp_cell_iterator_t citr;
  lisp_gc_color_t parent_color;
  lisp_gc_color_t child_color;
  size_t i;
  lisp_init_gc_reachable_iterator(&ritr);
  lisp_init_gc_stat(stat);
  stat->num_cycles = vm->num_cycles;
  stat->num_cons_pages = vm->num_cons_pages;
  for(i = 0; i < LISP_GC_NUM_CLASSES; i++)
  {
    stat->error_cons_lists[i] = 0;
    stat->error_object_lists[i] = 0;
    stat->num_conses[i] = _lisp_vm_check_object_list(&vm->cons_lists[i]->objects,
                                                     _lisp_dl_as_coll_list_cons,
                                                     &stat->error_cons_lists[i]);
    stat->num_objects[i] = _lisp_vm_check_object_list(&vm->object_lists[i]->objects,
                                                      _lisp_dl_as_coll_list_object,
                                                      &stat->error_object_lists[i]);
  }
  stat->num_root =
    stat->num_conses[LISP_GC_WHITE_ROOT] +
    stat->num_conses[LISP_GC_GREY_ROOT] +
    stat->num_conses[LISP_GC_BLACK_ROOT] +
    stat->num_objects[LISP_GC_WHITE_ROOT] +
    stat->num_objects[LISP_GC_GREY_ROOT] +
    stat->num_objects[LISP_GC_BLACK_ROOT];

  stat->num_allocated =
    stat->num_root +
    stat->num_conses[LISP_GC_WHITE] +
    stat->num_conses[LISP_GC_GREY] +
    stat->num_conses[LISP_GC_BLACK] +
    stat->num_objects[LISP_GC_WHITE] +
    stat->num_objects[LISP_GC_GREY] +
    stat->num_objects[LISP_GC_BLACK];

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
      if(LISP_STORAGE_ID(citr.child->type_id) == LISP_STORAGE_CONS ||
         LISP_STORAGE_ID(citr.child->type_id) == LISP_STORAGE_COMPLEX)
      {
        child_color = lisp_get_cell_color(citr.child);
        if((parent_color == LISP_GC_BLACK ||
            parent_color == LISP_GC_BLACK_ROOT) &&
           (child_color == LISP_GC_WHITE ||
            child_color == LISP_GC_WHITE_ROOT))
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
