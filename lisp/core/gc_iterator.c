/******************************************************************************
Copyright (c) 2020, Stefan Wolfsheimer

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
#include "gc_iterator.h"
#include "gc.h"
#include "tid.h"

lisp_gc_collectible_list_t * _get_color_map_list(lisp_gc_t * gc, int index)
{
  lisp_gc_color_map_t * gc_map;
  if(index < LISP_GC_NUM_COLORS * 2)
  {
    gc_map = &gc->cons_color_map;
  }
  else
  {
    gc_map = &gc->object_color_map;
    index-= LISP_GC_NUM_COLORS * 2;
  }
  switch(index)
  {
  case 0:
    return gc_map->white_root;
    break;
  case 1:
    return gc_map->grey_root;
    break;
  case 2:
    return gc_map->black_root;
    break;
  case 3:
    return gc_map->white;
    break;
  case 4:
    return gc_map->grey;
    break;
  case 5:
    return gc_map->black;
    break;
  }
  return NULL;
}

int lisp_gc_is_valid(lisp_gc_iterator_t * itr)
{
  return (itr->current_index < LISP_GC_NUM_COLORS * 4);
}

void lisp_gc_first(lisp_gc_t * gc, lisp_gc_iterator_t * itr)
{
  lisp_gc_collectible_list_t * lst;
  itr->current_item = NULL;
  for(itr->current_index = 0;
      itr->current_index < LISP_GC_NUM_COLORS * 4;
      itr->current_index++)
  {
    lst = _get_color_map_list(gc, itr->current_index);
    if(!lisp_dl_list_empty(&lst->objects))
    {
      itr->current_item = lst->objects.first;
      /* @todo other types if i >= LISP_GC_NUM_COLOR * 2*/
      itr->cell.type_id = LISP_TID_CONS;
      itr->cell.data.obj = (((char*) itr->current_item) +
                            sizeof(lisp_dl_item_t) +
                            sizeof(lisp_complex_object_t));
      return;
    }
  }
}

int lisp_gc_next(lisp_gc_t * gc, lisp_gc_iterator_t * itr)
{
  if(lisp_gc_is_valid(itr))
  {
    itr->current_item = itr->current_item->next;
    if(itr->current_item)
    {
      return 1;
    }
    else
    {
      lisp_gc_collectible_list_t * lst;
      itr->current_index++;
      for(;
          itr->current_index < LISP_GC_NUM_COLORS * 4;
          itr->current_index++)
      {
        lst = _get_color_map_list(gc, itr->current_index);
        if(!lisp_dl_list_empty(&lst->objects))
        {
          itr->current_item = lst->objects.first;
          return 1;
        }
      }
    }
  }
  return 0;
}

