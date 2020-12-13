#include "gc.h"
#include "error.h"
#include <lisp/util/xmalloc.h>
#include <lisp/core/cons.h>

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

static void _init_color_map_lists(lisp_gc_collectible_list_t * lists[],
                                  lisp_gc_color_map_t * gc_map)
{
  lists[0] = gc_map->white_root;
  lists[1] = gc_map->grey_root;
  lists[2] = gc_map->black_root;
  lists[3] = gc_map->white;
  lists[4] = gc_map->grey;
  lists[5] = gc_map->black;
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

/*****************************************************************************
 lisp_cons_t cast functions
 ****************************************************************************/
inline static lisp_collectible_object_t *
_lisp_cons_as_collectible_object(const lisp_cons_t * cons)
{
  return (lisp_collectible_object_t*)(
                                      ((char*)cons) -
                                      sizeof(lisp_collectible_object_t));
}

inline static lisp_dl_item_t *
_lisp_cons_as_dl_item(const lisp_cons_t * cons)
{
  return (lisp_dl_item_t*) (((char*)cons) -
                            sizeof(lisp_collectible_object_t) -
                            sizeof(lisp_dl_item_t));
}

/*****************************************************************************
 lisp_gc_t constructor / destructor
 ****************************************************************************/
int lisp_init_gc(lisp_gc_t * gc)
{
  int ret = lisp_init_color_map(&gc->cons_color_map);
  if(ret != LISP_OK)
  {
    return ret;
  }
  gc->cons_pages = NULL;
  lisp_init_dl_list(&gc->recycled_conses);
  gc->num_cons_pages = 0;
  gc->cons_pos = DEFAULT_PAGE_SIZE;
  gc->cons_page_size = DEFAULT_PAGE_SIZE;
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
                                         lisp_gc_collectible_list_t * list)
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
                                      sizeof(lisp_collectible_object_t) +
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
                               sizeof(lisp_collectible_object_t) +
                               sizeof(lisp_cons_t)) *
                              gc->cons_pos++));
  }
  lisp_dl_list_append(&list->objects, item);
  (
   (lisp_collectible_object_t*)
   (((char*)item) + sizeof(lisp_dl_item_t))
  )->lst = list;
  return (lisp_cons_t*)(((char*) item) +
                        sizeof(lisp_dl_item_t) +
                        sizeof(lisp_collectible_object_t));
}

lisp_cons_t * lisp_gc_alloc_cons(lisp_gc_t * gc)
{
  lisp_cons_t * cons = _lisp_gc_alloc_cons(gc, gc->cons_color_map.white);
  cons->ref_count = 0u;
  return cons;
}

lisp_cons_t * lisp_gc_alloc_root_cons(lisp_gc_t * gc)
{
  lisp_cons_t * cons = _lisp_gc_alloc_cons(gc, gc->cons_color_map.white_root);
  cons->ref_count = 1u;
  return cons;
}

void lisp_gc_free_cons(lisp_gc_t * gc, lisp_cons_t * cons)
{
  lisp_dl_list_remove(&_lisp_cons_as_collectible_object(cons)->lst->objects,
                      _lisp_cons_as_dl_item(cons));
  lisp_dl_list_append(&gc->recycled_conses,
                      _lisp_cons_as_dl_item(cons));
}

/****************************************************************************
 lisp_cons_t properties and GC statistics
 ****************************************************************************/
lisp_gc_color_t lisp_cons_get_color(const lisp_cons_t * cons)
{
  return _lisp_cons_as_collectible_object(cons)->lst->color;
}

short int lisp_cons_is_root(const lisp_cons_t * cons)
{
  return _lisp_cons_as_collectible_object(cons)->lst->is_root;
}

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
 lisp_cons_t iteration
 ****************************************************************************/
lisp_gc_cons_iterator_t lisp_gc_first_cons(lisp_gc_t * gc)
{
  lisp_gc_collectible_list_t * lists[LISP_GC_NUM_COLORS * 2];
  size_t i;
  lisp_gc_cons_iterator_t itr;
  itr.current_list = NULL;
  itr.current_item = NULL;
  _init_color_map_lists(lists, &gc->cons_color_map);
  itr.is_valid = 0;
  for(i = 0; i < LISP_GC_NUM_COLORS * 2; i++)
  {
    if(!lisp_dl_list_empty(&lists[i]->objects))
    {
      itr.current_list = lists[i];
      itr.current_item = itr.current_list->objects.first;
      itr.is_valid = 1;
      itr.cons = (lisp_cons_t*)(((char*) itr.current_item) +
                                sizeof(lisp_dl_item_t) +
                                sizeof(lisp_collectible_object_t));
      break;
    }
  }
  return itr;
}

void lisp_gc_next_cons(lisp_gc_t * gc, lisp_gc_cons_iterator_t * itr)
{
  lisp_gc_collectible_list_t * lists[LISP_GC_NUM_COLORS * 2];
  size_t i;
  _init_color_map_lists(lists, &gc->cons_color_map);
  if(itr->current_list)
  {
    itr->current_item = itr->current_item->next;
    if(!itr->current_item)
    {
      itr->current_item = NULL;
      itr->is_valid = 0;
      i = 0;
      while(lists[i] != itr->current_list)
      {
        ++i;
      }
      ++i;
      for(; i < LISP_GC_NUM_COLORS * 2; i++)
      {
        if(!lisp_dl_list_empty(&lists[i]->objects))
        {
          itr->current_list = lists[i];
          itr->current_item = itr->current_list->objects.first;
          itr->is_valid = 1;
          itr->cons = (lisp_cons_t*)(((char*) itr->current_item) +
                                     sizeof(lisp_dl_item_t) +
                                     sizeof(lisp_collectible_object_t)
                                     );
          break;
        }
      }
    }
    else
    {
      itr->cons = (lisp_cons_t*)(((char*) itr->current_item) +
                                 sizeof(lisp_dl_item_t) +
                                 sizeof(lisp_collectible_object_t)
                                 );
    }
  }
}

/****************************************************************************
 lisp_gc_t consistency checks and dump
 ****************************************************************************/
int lisp_gc_check(lisp_gc_t * gc)
{
  /* @todo: check consistency of children */
  lisp_gc_cons_iterator_t itr;
  size_t num_conses = 0;
  for(itr = lisp_gc_first_cons(gc);
      itr.is_valid;
      lisp_gc_next_cons(gc, &itr))
  {
    num_conses++;
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
  lisp_gc_cons_iterator_t itr;
  size_t num_conses = 0;
  for(itr = lisp_gc_first_cons(gc);
      itr.is_valid;
      lisp_gc_next_cons(gc, &itr))
  {
    num_conses++;
    fprintf(fp, "CONS %p\n", itr.cons);
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
