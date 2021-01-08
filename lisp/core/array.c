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
#include "array.h"
#include "cell.h"
#include "cell_iterator.h"
#include "error.h"
#include "tid.h"
#include <assert.h>

static int _array_destructor(void * ptr)
{
  return LISP_OK;
}

static int _array_first_child(lisp_cell_iterator_t * itr)
{
  assert(lisp_is_array(itr->parent));
  lisp_array_t* arr = lisp_as_array(itr->parent);
  lisp_cell_t* data = (lisp_cell_t*) &arr[1];
  itr->index = 0;
  if(arr->size == 0)
  {
    itr->child = NULL;
  }
  else
  {
    itr->child = data;
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
  lisp_cell_t* data = (lisp_cell_t*) &arr[1];
  itr->index++;
  if(itr->index < arr->size)
  {
    itr->child = data + itr->index;
  }
  else
  {
    itr->child = NULL;
  }
  return (itr->child != NULL);
}

int lisp_init_array_type(struct lisp_type_t * t)
{
  t->lisp_destructor_ptr = _array_destructor;
  t->lisp_cell_first_child_ptr = _array_first_child;
  t->lisp_cell_child_iterator_is_valid_ptr = _array_iterator_is_valid;
  t->lisp_cell_next_child_ptr = _array_next_child;
  return LISP_OK;

}
