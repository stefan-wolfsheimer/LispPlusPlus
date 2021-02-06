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

lisp_cell_t lisp_nil = { type_id : LISP_TID_NIL };

int lisp_is_null(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_NULL);
}

int lisp_is_nil(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_TID_NIL);
}

int lisp_is_atomic(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_ATOM);
}

int lisp_is_cow_object(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_COW_OBJECT);
}

int lisp_is_object(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_OBJECT);
}

int lisp_is_cons(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_CONS);
}

int lisp_is_complex(const lisp_cell_t * cell)
{
  return (LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_COMPLEX);
}

struct lisp_complex_object_t * lisp_as_complex_object(const lisp_cell_t * cell)
{
  if(LISP_STORAGE_ID(cell->type_id) == LISP_STORAGE_COMPLEX)
  {
    return (struct lisp_complex_object_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
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
  switch(LISP_STORAGE_ID(cell->type_id))
  {
  case LISP_STORAGE_CONS:
    return ((lisp_cons_t*)cell->data.obj)->ref_count;

  case LISP_STORAGE_COMPLEX:
    return ((lisp_complex_object_t*)cell->data.obj)[-1].ref_count;

  default:
    return 0;
  }
}

int lisp_is_root_cell(const lisp_cell_t * cell)
{
  switch(LISP_STORAGE_ID(cell->type_id))
  {
  case LISP_STORAGE_CONS:
    return ((lisp_cons_t*)cell->data.obj)->gc_list->is_root;

  case LISP_STORAGE_COMPLEX:
    return ((lisp_complex_object_t*)cell->data.obj)[-1].gc_list->is_root;

  default:
    return 0;

  }
}

lisp_gc_color_t lisp_get_cell_color(const lisp_cell_t * cell)
{
  switch(LISP_STORAGE_ID(cell->type_id))
  {
  case LISP_STORAGE_CONS:
    return ((lisp_cons_t*)cell->data.obj)->gc_list->color;

  case LISP_STORAGE_COMPLEX:
    return ((lisp_complex_object_t*)cell->data.obj)[-1].gc_list->color;

  default:
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

inline static lisp_dl_item_t * _lisp_complex_object_as_dl_item(const lisp_complex_object_t * obj)
{
  return (lisp_dl_item_t*) (((lisp_dl_item_t*)obj) - 1);
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
  switch(LISP_STORAGE_ID(cell->type_id))
  {
  case LISP_STORAGE_NULL:
  case LISP_STORAGE_ATOM:
    cell->type_id = LISP_TID_NIL;
    cell->data.obj = NULL;
    return LISP_OK;

  case LISP_STORAGE_COW_OBJECT:
    return LISP_NOT_IMPLEMENTED;

  case LISP_STORAGE_OBJECT:
    return LISP_NOT_IMPLEMENTED;

  case LISP_STORAGE_CONS:
    _lisp_cons_unset((lisp_cons_t*)cell->data.obj);
    cell->type_id = LISP_TID_NIL;
    cell->data.obj = NULL;
    return LISP_OK;

  case LISP_STORAGE_COMPLEX:
    _lisp_complex_unset(((lisp_complex_object_t*)cell->data.obj) - 1);
    cell->type_id = LISP_TID_NIL;
    cell->data.obj = NULL;
    return LISP_OK;

  default:
    return LISP_NOT_IMPLEMENTED;
  }
  return LISP_NOT_IMPLEMENTED;
}

/*****************************************************************************
 * internal functions
 *****************************************************************************/
static inline void _lisp_cons_grey(lisp_cons_t * cons)
{
  if(cons->gc_list->grey_elements != NULL)
  {
    lisp_dl_list_remove(&cons->gc_list->objects,
                        _lisp_cons_as_dl_item(cons));
    cons->gc_list = cons->gc_list->grey_elements;
    lisp_dl_list_append(&cons->gc_list->objects,
                        _lisp_cons_as_dl_item(cons));
  }
}

static inline void _lisp_complex_object_grey(lisp_complex_object_t * obj)
{
  if(obj->gc_list->grey_elements != NULL)
  {
    lisp_dl_list_remove(&obj->gc_list->objects,
                        _lisp_complex_object_as_dl_item(obj));
    obj->gc_list = obj->gc_list->grey_elements;
    lisp_dl_list_append(&obj->gc_list->objects,
                        _lisp_complex_object_as_dl_item(obj));
  }
}


int lisp_init_child_cell(lisp_cell_t * target,
                         const lisp_cell_t * source)
{
  /*@todo unit test coverage */
  if(source)
  {
    switch(LISP_STORAGE_ID(source->type_id))
    {
    case LISP_STORAGE_ATOM:
      target->data = source->data;
    case LISP_STORAGE_NULL:
      target->type_id = source->type_id;
      return LISP_OK;
      break;
    case LISP_STORAGE_COW_OBJECT:
      return LISP_NOT_IMPLEMENTED;
      break;
    case LISP_STORAGE_OBJECT:
      return LISP_NOT_IMPLEMENTED;
      break;
    case LISP_STORAGE_CONS:
      /* ensure that child is not white. */
      _lisp_cons_grey((lisp_cons_t*)source->data.obj);
      target->type_id = source->type_id;
      target->data = source->data;
      return LISP_OK;
    case LISP_STORAGE_COMPLEX:
      /* ensure that child is not white */
      _lisp_complex_object_grey((lisp_complex_object_t*)source->data.obj - 1);
      target->type_id = source->type_id;
      target->data = source->data;
      return LISP_OK;
      break;
    }
    /*@todo implement other types */
    return LISP_NOT_IMPLEMENTED;
  }
  else
  {
    target->type_id = LISP_TID_NIL;
  }
  return LISP_OK;
}

int lisp_unset_child_cell(lisp_cell_t * target)
{
  switch(LISP_STORAGE_ID(target->type_id))
  {
  case LISP_STORAGE_ATOM:
  case LISP_STORAGE_NULL:
    target->type_id = LISP_TID_NIL;
    return LISP_OK;
    break;
  case LISP_STORAGE_COW_OBJECT:
    return LISP_NOT_IMPLEMENTED;
    break;
  case LISP_STORAGE_OBJECT:
    return LISP_NOT_IMPLEMENTED;
    break;
  case LISP_STORAGE_CONS:
    target->type_id = LISP_TID_NIL;
    target->data.obj = NULL;
    return LISP_OK;
  case LISP_STORAGE_COMPLEX:
    target->type_id = LISP_TID_NIL;
    target->data.obj = NULL;
    return LISP_OK;
    break;
  default:
    return LISP_NOT_IMPLEMENTED;
  }
  return LISP_OK;
}

