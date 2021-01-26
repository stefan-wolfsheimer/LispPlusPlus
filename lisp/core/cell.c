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
#include <lisp/util/hash_table.h>
#include <lisp/util/murmur_hash3.h>
#include "error.h"
#include "cell.h"
#include "tid.h"
#include "cons.h"
#include "gc_color_map.h"
#include "complex_object.h"

lisp_cell_t lisp_nil =
{
 type_id : LISP_TID_NIL
};

int lisp_is_atomic(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_ATOM_TID(cell->type_id);
}

int lisp_is_object(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_OBJECT_TID(cell->type_id);
}

int lisp_is_reference(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_REFERENCE_TID(cell->type_id);
}

int lisp_is_complex(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_COMPLEX_TID(cell->type_id);
}

struct lisp_complex_object_t * lisp_as_complex_object(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return (struct lisp_complex_object_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
}

int lisp_is_cons(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_CONS_TID(cell->type_id);
}

struct lisp_cons_t * lisp_as_cons(lisp_cell_t * cell)
{
  if(LISP_IS_CONS_TID(cell->type_id))
  {
    return (struct lisp_cons_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
}

int lisp_is_array(const lisp_cell_t * cell)
{
  return cell->type_id == LISP_TID_ARRAY;
}

struct lisp_array_t * lisp_as_array(lisp_cell_t * cell)
{
  if(cell->type_id == LISP_TID_ARRAY)
  {
    return (struct lisp_array_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
}

size_t lisp_get_ref_count(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    return ((lisp_cons_t*)cell->data.obj)->ref_count;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].ref_count;
  }
  else
  {
    return 0;
  }
}

int lisp_is_root_cell(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    return ((lisp_cons_t*)cell->data.obj)->gc_list->is_root;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].gc_list->is_root;
  }
  else
  {
    return 1;
  }
}

lisp_gc_color_t lisp_get_cell_color(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    return ((lisp_cons_t*)cell->data.obj)->gc_list->color;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].gc_list->color;
  }
  else
  {
    return LISP_GC_NO_COLOR;
  }
}

/******************************************************************************
 * lisp_unset
 ******************************************************************************/
inline static lisp_dl_item_t * _lisp_cons_as_dl_item(const lisp_cons_t * cons)
{
  return (lisp_dl_item_t*) (((char*)cons) - sizeof(lisp_dl_item_t));
}

static void _lisp_cons_unset(lisp_cons_t * cons)
{
  assert(cons->ref_count > 0);
  if(!(--cons->ref_count))
  {
    lisp_dl_list_remove(&cons->gc_list->objects,
                        _lisp_cons_as_dl_item(cons));
    cons->gc_list = cons->gc_list->other_elements;
    lisp_dl_list_append(&cons->gc_list->objects,
                        _lisp_cons_as_dl_item(cons));
  }
}

static void _lisp_complex_unset(lisp_complex_object_t * obj)
{
  assert(obj->ref_count > 0);
  if(!(--obj->ref_count))
  {
    lisp_dl_list_remove(&obj->gc_list->objects, ((lisp_dl_item_t*)obj) - 1);
    obj->gc_list = obj->gc_list->other_elements;
    lisp_dl_list_append(&obj->gc_list->objects, ((lisp_dl_item_t*)obj) - 1);
  }
}

int lisp_unset(lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_ATOM_TID(cell->type_id))
  {
    cell->type_id = LISP_TID_NIL;
    cell->data.obj = NULL;
  }
  else if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    _lisp_cons_unset((lisp_cons_t*)cell->data.obj);
    return LISP_OK;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    _lisp_complex_unset(((lisp_complex_object_t*)cell->data.obj) - 1);
    return LISP_OK;
  }
  else
  {
    return LISP_NOT_IMPLEMENTED;
  }
  return LISP_OK;
}

