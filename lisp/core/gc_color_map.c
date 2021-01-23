#include <lisp/util/xmalloc.h>
#include <lisp/core/error.h>
#include "gc_color_map.h"

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


int lisp_init_color_map(lisp_gc_color_map_t * map)
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

int lisp_free_color_map(lisp_gc_color_map_t * map)
{
  FREE(map->white_root);
  FREE(map->grey_root);
  FREE(map->black_root);
  FREE(map->white);
  FREE(map->grey);
  FREE(map->black);
  return LISP_OK;
}

