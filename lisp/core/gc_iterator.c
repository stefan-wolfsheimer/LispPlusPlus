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
#include "cell_iterator.h"
#include "cell_hash_table.h"
#include "gc.h"
#include "tid.h"
#include "error.h"

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

int lisp_gc_iterator_is_valid(lisp_gc_iterator_t * itr)
{
  return (itr->current_index < LISP_GC_NUM_COLORS * 4);
}

static void _init_cell(lisp_gc_iterator_t * itr)
{
  if(itr->current_index < LISP_GC_NUM_COLORS * 2)
  {
    itr->cell.type_id = LISP_TID_CONS;
    itr->cell.data.obj = (((char*) itr->current_item) +
                          sizeof(lisp_dl_item_t));
  }
  else
  {
    lisp_complex_object_t * obj =
      (lisp_complex_object_t *)
      (((char*) itr->current_item) + sizeof(lisp_dl_item_t));
    itr->cell.type_id = obj->type_id;
    itr->cell.data.obj = (obj + 1);
  }
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
      _init_cell(itr);
      return;
    }
  }
}

int lisp_gc_next(lisp_gc_t * gc, lisp_gc_iterator_t * itr)
{
  if(lisp_gc_iterator_is_valid(itr))
  {
    itr->current_item = itr->current_item->next;
    if(itr->current_item)
    {
      _init_cell(itr);
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
          _init_cell(itr);
          return 1;
        }
      }
    }
  }
  return 0;
}

/* reachable iterator */
int lisp_init_gc_reachable_iterator(lisp_gc_reachable_iterator_t * itr)
{
  itr->cell = lisp_nil;
  int ret = lisp_init_cell_hash_table(&itr->todo);
  if(ret != LISP_OK)
  {
    return ret;
  }
  ret = lisp_init_cell_hash_table(&itr->root);
  return ret;
}

void lisp_free_gc_reachable_iterator(lisp_gc_reachable_iterator_t * itr)
{
  lisp_free_cell_hash_table(&itr->todo);
  lisp_free_cell_hash_table(&itr->root);
}

void lisp_gc_reachable_first(struct lisp_gc_t * gc,
                             lisp_gc_reachable_iterator_t * itr)
{
  int i;
  lisp_gc_collectible_list_t * lst;
  lisp_dl_item_t * item;
  lisp_cell_t cell;
  itr->entry = NULL;
  for(i = 0; i < 3; i++)
  {
    lst = _get_color_map_list(gc, i);
    item = lst->objects.first;
    while(item)
    {
      cell.type_id = LISP_TID_CONS;
      cell.data.obj = (((char*) item) + sizeof(lisp_dl_item_t));
      lisp_cell_hash_table_set(&itr->todo, &cell);
      item = item->next;
    }
  }
  /* @todo other objects */
  for(i = 0; i < 3; i++)
  {
    lst = _get_color_map_list(gc, LISP_GC_NUM_COLORS * 2 + i);
  }
  itr->entry = HASH_TABLE_FIRST(&itr->todo);
  if(itr->entry != NULL)
  {
    itr->cell = *HASH_TABLE_DATA(itr->entry, lisp_cell_t);
  }
  else
  {
    itr->cell = lisp_nil;
  }
}

int lisp_gc_reachable_iterator_is_valid(lisp_gc_reachable_iterator_t * itr)
{
  return (itr->entry != NULL);
}

int lisp_gc_reachable_next(struct lisp_gc_t * gc,
                           lisp_gc_reachable_iterator_t * itr)
{
  if(itr->entry != NULL)
  {
    lisp_cell_iterator_t citr;
    lisp_cell_t * cell;
    cell = lisp_cell_hash_table_set(&itr->root,
                                    HASH_TABLE_DATA(itr->entry, lisp_cell_t));
    lisp_cell_hash_table_remove(&itr->todo,
                                HASH_TABLE_DATA(itr->entry, lisp_cell_t));
    for(lisp_first_child(cell, &citr);
        lisp_cell_iterator_is_valid(&citr);
        lisp_cell_next_child(&citr))
    {
      if(!lisp_cell_in_hash(&itr->todo, citr.child) &&
         !lisp_cell_in_hash(&itr->root, citr.child))
      {
        if(LISP_IS_STORAGE_COMPLEX_TID(citr.child->type_id))
        {
          lisp_cell_hash_table_set(&itr->todo, citr.child);
        }
      }
    }
    itr->entry = HASH_TABLE_FIRST(&itr->todo);
    if(itr->entry != NULL)
    {
      itr->cell = *HASH_TABLE_DATA(itr->entry, lisp_cell_t);
      return 1;
    }
    else
    {
      itr->cell = lisp_nil;
      return 0;
    }
  }
  else
  {
    itr->cell = lisp_nil;
    return 0;
  }
}
