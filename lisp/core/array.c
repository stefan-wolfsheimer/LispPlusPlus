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
#include "vm.h"
#include "array.h"
#include "cell.h"
#include "cell_iterator.h"
#include "error.h"
#include "tid.h"
#include <assert.h>

static int _lisp_array_init_cells(lisp_cell_t * target,
                                  size_t begin,
                                  size_t end,
                                  lisp_cell_t * value);


int lisp_make_array(struct lisp_vm_t * vm,
                    struct lisp_cell_t * cell,
                    size_t n,
                    struct lisp_cell_t * value)
{
  lisp_cell_t * cells;
  cell->type_id = LISP_TID_ARRAY;
  cell->data.obj = lisp_vm_alloc_root_complex_object(vm,
                                                     LISP_TID_ARRAY,
                                                     sizeof(lisp_array_t));
  if(!cell->data.obj)
  {
    return LISP_BAD_ALLOC;
  }
  cells = MALLOC(sizeof(lisp_cell_t) * n);
  ((lisp_array_t*)cell->data.obj)->size = n;
  ((lisp_array_t*)cell->data.obj)->data = cells;
  ((lisp_array_t*)cell->data.obj)->gc_pos = 0;
  return _lisp_array_init_cells(cells, 0, n, value);
}

/******************************************************************************
 * array modification
 ******************************************************************************/
static int _lisp_array_init_cells(lisp_cell_t * cells,
                                  size_t begin,
                                  size_t end,
                                  lisp_cell_t * value)
{
  size_t i;
  if(value)
  {
    /* @todo */
    return LISP_NOT_IMPLEMENTED;
  }
  else
  {
    for(i=begin; i < end; i++)
    {
      cells[i].type_id = LISP_TID_NIL;
    }
    return LISP_OK;
  }
}

static int _lisp_array_unset_cells(lisp_cell_t * cells,
                                   size_t begin,
                                   size_t end)
{
  size_t i;
  for(i=begin; i < end; i++)
  {
    switch(LISP_STORAGE_ID(cells->type_id))
    {
    case LISP_STORAGE_ATOM:
    case LISP_STORAGE_NULL:
      cells->type_id = LISP_TID_NIL;
      break;
    case LISP_STORAGE_COW_OBJECT:
      return LISP_NOT_IMPLEMENTED;
      break;
    case LISP_STORAGE_OBJECT:
      return LISP_NOT_IMPLEMENTED;
      break;
    case LISP_STORAGE_CONS:
      cells->type_id = LISP_TID_NIL;
      cells->data.obj = NULL;
      break;
    case LISP_STORAGE_COMPLEX:
      cells->type_id = LISP_TID_NIL;
      cells->data.obj = NULL;
      break;
    default:
      return LISP_NOT_IMPLEMENTED;
    }
    cells++;
  }
  return LISP_OK;
}

int lisp_array_resize(struct lisp_vm_t * vm,
                      struct lisp_array_t * array,
                      size_t n,
                      struct lisp_cell_t * value)
{
  /* @todo check gc_pos */
  int ret;
  if(n < array->size)
  {
    ret = _lisp_array_unset_cells(array->data, n, array->size);
    if(ret != LISP_OK)
    {
      return ret;
    }
    array->data = REALLOC(array->data, n*sizeof(lisp_cell_t));
    array->size = n;
  }
  else if(n > array->size)
  {
    array->data = REALLOC(array->data, n*sizeof(lisp_cell_t));
    ret = _lisp_array_init_cells(array->data,
                                 array->size,
                                 n,
                                 value);
    if(ret != LISP_OK)
    {
      return ret;
    }
    array->size = n;
  }
  return LISP_OK;
}


/* typdefinition functions */
static int _array_destructor(void * ptr)
{
  FREE(((lisp_array_t*)ptr)->data);
  return LISP_OK;
}

static int _array_first_child(lisp_cell_iterator_t * itr)
{
  assert(lisp_is_array(itr->parent));
  lisp_array_t * arr = lisp_as_array(itr->parent);
  itr->index = 0;
  if(arr->size == 0)
  {
    itr->child = NULL;
  }
  else
  {
    itr->child = arr->data;
  }
  return LISP_OK;
}

static int _array_iterator_is_valid(lisp_cell_iterator_t * itr)
{
  return (itr->child != NULL);
}

static int _array_next_child(lisp_cell_iterator_t * itr)
{
  assert(lisp_is_array(itr->parent));
  lisp_array_t* arr = lisp_as_array(itr->parent);
  itr->index++;
  if(itr->index < arr->size)
  {
    itr->child = arr->data + itr->index;
  }
  else
  {
    itr->child = NULL;
  }
  return (itr->child != NULL);
}

static lisp_cell_t * _array_grey_next_child_ptr(void * ptr)
{
  /* @todo handle step */
  if(((lisp_array_t*)ptr)->gc_pos >= ((lisp_array_t*)ptr)->size)
  {
    return NULL;
  }
  else
  {
    return ((lisp_array_t*)ptr)->data + ((lisp_array_t*)ptr)->gc_pos++;
  }
}

int lisp_init_array_type(struct lisp_type_t * t)
{
  t->lisp_destructor_ptr = _array_destructor;
  t->lisp_cell_first_child_ptr = _array_first_child;
  t->lisp_cell_child_iterator_is_valid_ptr = _array_iterator_is_valid;
  t->lisp_cell_next_child_ptr = _array_next_child;
  t->grey_next_child_ptr = _array_grey_next_child_ptr;
  return LISP_OK;
}
