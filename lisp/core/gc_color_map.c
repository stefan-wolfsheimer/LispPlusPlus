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
#include <lisp/core/error.h>
#include "gc_color_map.h"

/*****************************************************************************
 Helper functions
 ****************************************************************************/
static lisp_gc_collectible_list_t * _new_gc_collectible_list(lisp_gc_color_t c,
                                                             short int is_root,
                                                             struct lisp_vm_t * vm)
{
  lisp_gc_collectible_list_t * lst = MALLOC(sizeof(lisp_gc_collectible_list_t));
  if(lst)
  {
    lst->is_root = is_root;
    lisp_init_dl_list(&lst->objects);
    lst->color = c;
    lst->vm = vm;
  }
  return lst;
}


int lisp_init_color_map(lisp_gc_color_map_t * map,
                        struct lisp_vm_t * vm)
{
  if( (map->white = _new_gc_collectible_list(LISP_GC_WHITE, 0, vm)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->grey = _new_gc_collectible_list(LISP_GC_GREY, 0, vm)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->black = _new_gc_collectible_list(LISP_GC_BLACK, 0, vm)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->white_root = _new_gc_collectible_list(LISP_GC_WHITE, 1, vm)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->grey_root = _new_gc_collectible_list(LISP_GC_GREY, 1, vm)) == NULL)
  {
    return LISP_BAD_ALLOC;
  }
  if( (map->black_root = _new_gc_collectible_list(LISP_GC_BLACK, 1, vm)) == NULL)
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

