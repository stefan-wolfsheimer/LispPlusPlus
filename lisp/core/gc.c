#include "gc.h"
#include "gc_iterator.h"
#include "cell_iterator.h"
#include "error.h"
#include "tid.h"
#include <lisp/util/xmalloc.h>
#include <lisp/core/cons.h>
#include <lisp/util/hash_table.h>
#include <lisp/util/murmur_hash3.h>

#define DEFAULT_PAGE_SIZE 1024

/*****************************************************************************
 Helper functions
 ****************************************************************************/
static lisp_gc_collectible_list_t * _new_gc_collectible_list(lisp_gc_color_t c,
                                                             short int is_root)
{
  lisp_gc_collectible_list_t * lst = MALLOC(sizeof(lisp_gc_collectible_list_t));
  if(lst)
  {
    lst->is_root = is_root;
    lisp_init_dl_list(&lst->objects);
    lst->color = c;
  }
  return lst;
}

static int lisp_free_color_map(lisp_gc_color_map_t * map)
{
  FREE(map->white_root);
  FREE(map->grey_root);
  FREE(map->black_root);
  FREE(map->white);
  FREE(map->grey);
  FREE(map->black);
  return LISP_OK;
}

static int lisp_init_color_map(lisp_gc_color_map_t * map)
{
  if( (map->white = _new_gc_collectible_list(LISP_GC_WHITE, 0)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->grey = _new_gc_collectible_list(LISP_GC_GREY, 0)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->black = _new_gc_collectible_list(LISP_GC_BLACK, 0)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->white_root = _new_gc_collectible_list(LISP_GC_WHITE, 1)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->grey_root = _new_gc_collectible_list(LISP_GC_GREY, 1)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->black_root = _new_gc_collectible_list(LISP_GC_BLACK, 1)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  /*
   * we don't know if another object still refers to unrooted objects
   * -> never transition from root to white
   */
  map->white_root->other_elements = map->grey;
  map->grey_root->other_elements  = map->grey;
  map->black_root->other_elements = map->black;
  map->white->other_elements      = map->white_root;
  map->grey->other_elements       = map->white_root;
  map->black->other_elements      = map->white_root;

  map->white_root->grey_elements  = map->grey_root;
  map->grey_root->grey_elements   = NULL;
  map->black_root->grey_elements  = NULL;
  map->white->grey_elements       = map->grey;
  map->grey->grey_elements        = NULL;
  map->black->grey_elements       = NULL;

  map->white_root->to_elements    = map->grey_root;
  map->grey_root->to_elements     = map->black_root;
  map->black_root->to_elements    = NULL;
  map->white->to_elements         = map->grey;
  map->grey->to_elements          = map->black;
  map->black->to_elements         = NULL;

  return LISP_OK;
}

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

/*****************************************************************************
 cast functions
 ****************************************************************************/
inline static lisp_dl_item_t * _lisp_cons_as_dl_item(const lisp_cons_t * cons)
{
  return (lisp_dl_item_t*) (((char*)cons) - sizeof(lisp_dl_item_t));
}

/*****************************************************************************
 lisp_gc_t constructor / destructor
 ****************************************************************************/
int lisp_init_gc(lisp_gc_t * gc)
{
  int ret = lisp_load_static_types();
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_init_color_map(&gc->cons_color_map);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_init_color_map(&gc->object_color_map);
  if(ret != LISP_OK)
  {
    return ret;
  }
  gc->cons_pages = NULL;
  lisp_init_dl_list(&gc->recycled_conses);
  gc->num_cons_pages = 0;
  gc->cons_pos = DEFAULT_PAGE_SIZE;
  gc->cons_page_size = DEFAULT_PAGE_SIZE;
  gc->num_cycles = 0;
  gc->num_steps = 1;
  return LISP_OK;
}

int lisp_free_gc(lisp_gc_t * gc)
{
  int ret = lisp_free_color_map(&gc->cons_color_map);
  size_t i;
  for(i = 0; i < gc->num_cons_pages; i++)
  {
    FREE(gc->cons_pages[i]);
  }
  if(gc->cons_pages)
  {
    FREE(gc->cons_pages);
  }
  ret = lisp_erase_list(gc->object_color_map.white_root);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(gc->object_color_map.grey_root);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(gc->object_color_map.white_root);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(gc->object_color_map.white);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(gc->object_color_map.grey);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_erase_list(gc->object_color_map.white);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_free_color_map(&gc->object_color_map);
  return ret;
}

size_t lisp_gc_set_steps(lisp_gc_t * gc, size_t n)
{
  size_t ret = gc->num_steps;
  gc->num_steps = n;
  return ret;
}

/****************************************************************************
 lisp_cons_t  memory management functions
 ****************************************************************************/
int lisp_gc_set_cons_page_size(lisp_gc_t * gc, size_t page_size)
{
  if(gc->num_cons_pages)
  {
    return LISP_INVALID;
  }
  else
  {
    gc->cons_pos = page_size;
    gc->cons_page_size = page_size;
    return LISP_OK;
  }
}

static lisp_cons_t * _lisp_gc_alloc_cons(lisp_gc_t * gc,
                                         lisp_gc_collectible_list_t * list,
                                         size_t ref_count)
{
  lisp_dl_item_t * item;
  if(!lisp_dl_list_empty(&gc->recycled_conses))
  {
    item = gc->recycled_conses.first;
    lisp_dl_list_remove_first(&gc->recycled_conses);
  }
  else
  {
    if(gc->cons_pos >= gc->cons_page_size)
    {
      gc->current_cons_page = MALLOC((sizeof(lisp_dl_item_t) +
                                      sizeof(lisp_cons_t)) * gc->cons_page_size);
      gc->cons_pages = REALLOC(gc->cons_pages,
                               sizeof(void*) * (gc->num_cons_pages + 1));
      if(!gc->cons_pages)
      {
        return NULL;
      }
      gc->cons_pages[gc->num_cons_pages++] = gc->current_cons_page;
      gc->cons_pos = 0;
    }
    item = (lisp_dl_item_t*)(
                             (char*) gc->current_cons_page +
                             ((
                               sizeof(lisp_dl_item_t) +
                               sizeof(lisp_cons_t)) *
                              gc->cons_pos++));
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

lisp_cons_t * lisp_gc_alloc_cons(lisp_gc_t * gc)
{
  return _lisp_gc_alloc_cons(gc,
                             gc->cons_color_map.white,
                             0u);
}

lisp_cons_t * lisp_gc_alloc_root_cons(lisp_gc_t * gc)
{
  return _lisp_gc_alloc_cons(gc,
                             gc->cons_color_map.white_root,
                             1u);
}

static void * _lisp_gc_alloc_object(lisp_gc_t * gc,
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

void * lisp_gc_alloc_object(lisp_gc_t * gc, lisp_type_id_t tid, size_t size)
{
  return _lisp_gc_alloc_object(gc,
                               gc->object_color_map.white,
                               tid,
                               size,
                               0u);
}

void * lisp_gc_alloc_root_object(lisp_gc_t * gc, lisp_type_id_t tid, size_t size)
{
  return _lisp_gc_alloc_object(gc,
                               gc->object_color_map.white_root,
                               tid,
                               size,
                               1u);
}

void lisp_gc_free_cons(lisp_gc_t * gc, lisp_cons_t * cons)
{
  lisp_dl_list_remove(&cons->gc_list->objects,
                      _lisp_cons_as_dl_item(cons));
  lisp_dl_list_append(&gc->recycled_conses,
                      _lisp_cons_as_dl_item(cons));
}

/*****************************************************************************
 Garbage collector operation
 ****************************************************************************/
static inline short int _lisp_gc_cons_step(lisp_gc_t * gc)
{
  /* @todo implement function */
  return 0;
}

static inline short int _lisp_gc_object_step(lisp_gc_t * gc)
{
  /* @todo implement function */
  return 0;
}

void lisp_gc_step(lisp_gc_t * gc)
{
  short int swapable = 1;
  swapable &= _lisp_gc_cons_step(gc);
  swapable &= _lisp_gc_object_step(gc);
  if(swapable)
  {
    gc->num_cycles++;
    /*
      @todo implement swap
    */
  }
}

short int lisp_gc_cons_step(lisp_gc_t * gc)
{
  return _lisp_gc_cons_step(gc);
}

short int lisp_gc_object_step(lisp_gc_t * gc)
{
  return _lisp_gc_object_step(gc);
}

/****************************************************************************
 lisp_cons_t properties and GC statistics
 ****************************************************************************/
size_t lisp_gc_num_conses(lisp_gc_t * gc)
{
  return lisp_gc_num_allocated_conses(gc) - lisp_gc_num_recycled_conses(gc);
}

size_t lisp_gc_num_allocated_conses(lisp_gc_t * gc)
{
  if( gc->num_cons_pages )
  {
    return (gc->num_cons_pages - 1) * gc->cons_page_size + gc->cons_pos;
  }
  else
  {
    return 0;
  }
}

size_t lisp_gc_num_recycled_conses(lisp_gc_t * gc)
{
  return lisp_dl_list_size(&gc->recycled_conses);
}

/****************************************************************************
 lisp_gc_t consistency checks and dump
 ****************************************************************************/
int lisp_gc_check(lisp_gc_t * gc)
{
  /* @todo more checks */
  lisp_gc_stat_t stat;
  lisp_gc_iterator_t itr;
  size_t num_conses = 0;
  lisp_gc_get_stats(gc, &stat);
  if(stat.error_black_has_white_child)
  {
    return LISP_INVALID;
  }
  for(lisp_gc_first(gc, &itr);
      lisp_gc_iterator_is_valid(&itr);
      lisp_gc_next(gc, &itr))
  {
    if(LISP_IS_STORAGE_CONS_TID(itr.cell.type_id))
    {
      num_conses++;
    }
  }
  if(lisp_gc_num_allocated_conses(gc) !=
     (num_conses + lisp_gc_num_recycled_conses(gc)))
  {
    return LISP_INVALID;
  }
  return LISP_OK;
}

static void _lisp_gc_dump_humamn(FILE * fp, lisp_gc_t * gc)
{
  lisp_gc_iterator_t itr;
  size_t num_conses = 0;
  for(lisp_gc_first(gc, &itr);
      lisp_gc_iterator_is_valid(&itr);
      lisp_gc_next(gc, &itr))
  {
    if(LISP_IS_STORAGE_CONS_TID(itr.cell.type_id))
    {
      num_conses++;
    }
    fprintf(fp, "OBJ %p\n", itr.cell.data.obj);
  }
  fprintf(fp, "\n");
  fprintf(fp, "NUM CONSES:       %d\n", (int)num_conses);
  fprintf(fp, "RECYCLED CONSES:  %d\n", (int)lisp_gc_num_recycled_conses(gc));
  fprintf(fp, "ALLOCATED CONSES: %d\n", (int)lisp_gc_num_allocated_conses(gc));
}

void lisp_gc_dump(FILE * fp, lisp_gc_t * gc, int mode)
{
  switch(mode)
  {
  case LISP_GC_DUMP_HUMAN:
    _lisp_gc_dump_humamn(fp, gc);
    break;
  }
}

void lisp_gc_get_stats(lisp_gc_t * gc,
                       lisp_gc_stat_t * stat)
{
  lisp_gc_reachable_iterator_t ritr;
  lisp_cell_iterator_t citr;
  lisp_gc_color_t parent_color;
  lisp_init_gc_reachable_iterator(&ritr);
  stat->error_black_has_white_child = 0;
  stat->num_leaves = 0;
  stat->num_edges = 0;
  stat->num_leaves = 0;
  stat->num_cycles = gc->num_cycles;
  stat->num_root =
    lisp_dl_list_size(&gc->cons_color_map.white_root->objects) +
    lisp_dl_list_size(&gc->cons_color_map.grey_root->objects) +
    lisp_dl_list_size(&gc->cons_color_map.black_root->objects) +
    lisp_dl_list_size(&gc->object_color_map.white_root->objects) +
    lisp_dl_list_size(&gc->object_color_map.grey_root->objects) +
    lisp_dl_list_size(&gc->object_color_map.black_root->objects);
  stat->num_allocated =
    stat->num_root +
    lisp_dl_list_size(&gc->cons_color_map.white->objects) +
    lisp_dl_list_size(&gc->cons_color_map.grey->objects) +
    lisp_dl_list_size(&gc->cons_color_map.black->objects) +
    lisp_dl_list_size(&gc->object_color_map.white->objects) +
    lisp_dl_list_size(&gc->object_color_map.grey->objects) +
    lisp_dl_list_size(&gc->object_color_map.black->objects);
  stat->num_reachable = 0;
  for(lisp_gc_reachable_first(gc, &ritr);
      lisp_gc_reachable_iterator_is_valid(&ritr);
      lisp_gc_reachable_next(gc, &ritr))
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
  stat->num_recycled = lisp_dl_list_size(&gc->recycled_conses);
  stat->num_void = (gc->cons_page_size - gc->cons_pos);
  /*
    @todo implement
    size_t num_disposed
  */
  lisp_free_gc_reachable_iterator(&ritr);
}
