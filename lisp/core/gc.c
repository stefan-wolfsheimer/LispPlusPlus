#include "gc.h"
#include "error.h"
#include <lisp/util/xmalloc.h>
#include <lisp/core/cons.h>

#define DEFAULT_PAGE_SIZE 1024

/*****************************************************************************/
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

#include <stdio.h>

static lisp_cons_t * _lisp_gc_alloc_cons(lisp_gc_t * gc,
                                         lisp_gc_collectible_list_t * list)
{
  if(!lisp_dl_list_empty(&gc->recycled_conses))
  {
    lisp_dl_item_t * ret = gc->recycled_conses.first;
    ret++;
    lisp_dl_list_remove_first(&gc->recycled_conses);
    return (lisp_cons_t*)ret;
  }
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
  lisp_dl_item_t * item = (lisp_dl_item_t*)(
                                            (char*) gc->current_cons_page +
                                            ((
                                              sizeof(lisp_dl_item_t) +
                                              sizeof(lisp_collectible_object_t) +
                                              sizeof(lisp_cons_t)) *
                                             gc->cons_pos++));
  lisp_dl_list_append(&list->objects, item);
  (
   (lisp_collectible_object_t*)
   (((char*)item) + sizeof(lisp_dl_item_t))
   )->lst = list;

  return (lisp_cons_t*)(((char*) item) +
                        sizeof(lisp_dl_item_t) +
                        sizeof(lisp_collectible_object_t)
                        );
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
  lisp_dl_list_append(&gc->recycled_conses,
                      (lisp_dl_item_t*) (((char*)cons) -
                                         sizeof(lisp_collectible_object_t) -
                                         sizeof(lisp_dl_item_t)));
}
