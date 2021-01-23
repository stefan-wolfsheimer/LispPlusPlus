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
  int ret = lisp_free_color_map(&vm->cons_color_map);
  size_t i;
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
 lisp_vm_t garbage collector operation
 ****************************************************************************/
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
               &vm->recycled_conses,
               item,
               &ritr.root);
  _move_conses(vm->cons_color_map.white,
               &vm->recycled_conses,
               vm->cons_color_map.grey->objects.first,
               &ritr.root);
  vm->cons_color_map.grey->objects.first = NULL;
  vm->cons_color_map.grey->objects.last = NULL;
  _move_conses(vm->cons_color_map.white,
               &vm->recycled_conses,
               vm->cons_color_map.black->objects.first,
               &ritr.root);
  vm->cons_color_map.black->objects.first = NULL;
  vm->cons_color_map.black->objects.last = NULL;

  /* objects */
  _move_object_list(vm->object_color_map.white_root,
                    vm->object_color_map.grey_root);
  _move_object_list(vm->object_color_map.white_root,
                    vm->object_color_map.black_root);
  lisp_free_gc_reachable_iterator(&ritr);
  vm->num_cycles++;
  return LISP_OK;
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
    if(LISP_IS_STORAGE_ATOM_TID(source->type_id))
    {
      target->type_id = source->type_id;
      target->data = source->data;
    }
    else if(LISP_IS_STORAGE_CONS_TID(source->type_id))
    {
      /* ensure that child is not white. */
      lisp_cons_grey((lisp_cons_t*)source->data.obj);
      target->type_id = source->type_id;
      target->data = source->data;
      return LISP_OK;
    }
    else
    {
      /*@todo implement other types */
      return LISP_NOT_IMPLEMENTED;
    }
  }
  else
  {
    target->type_id = LISP_TID_NIL;
  }
  return LISP_OK;
}

static void * _lisp_gc_alloc_object(lisp_vm_t * vm,
                                    lisp_gc_collectible_list_t * list,
                                    lisp_type_id_t tid,
                                    size_t size,
                                    size_t ref_count)
{
  lisp_dl_item_t * item;
  lisp_complex_object_t * obj;
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

int lisp_make_array(lisp_vm_t * vm, lisp_cell_t * cell, size_t n)
{
  lisp_cell_t * cells;
  size_t i;
  cell->type_id = LISP_TID_ARRAY;
  cell->data.obj = _lisp_gc_alloc_object(vm,
                                         vm->object_color_map.white_root,
                                         LISP_TID_ARRAY,
                                         sizeof(lisp_array_t) + sizeof(lisp_cell_t) * n,
                                         1u);
  ((lisp_array_t*)cell->data.obj)->size = n;
  if(!cell->data.obj)
  {
    return LISP_BAD_ALLOC;
  }
  cells = (lisp_cell_t*)&(((lisp_array_t*)cell->data.obj)[1]);
  for(i=0; i < n; i++)
  {
    cells[i].type_id = LISP_TID_NIL;
  }
  return LISP_OK;
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
     (num_conses + lisp_dl_list_size(&vm->recycled_conses)))
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
  /*
    @todo implement
    size_t num_disposed
  */
  lisp_free_gc_reachable_iterator(&ritr);
}
