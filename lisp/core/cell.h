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
#ifndef __LISP_CELL_H__
#define __LISP_CELL_H__
#include <stddef.h>

typedef unsigned short int lisp_type_id_t;

typedef struct lisp_cell_t
{
  lisp_type_id_t type_id;
  union
  {
    void * obj;
  } data;
} lisp_cell_t;

extern lisp_cell_t lisp_nil;

/**
 * Check for storage class of cell.
 *
 * @return true if cell has atomic storage class
 */
int lisp_is_atomic(lisp_cell_t * cell);

/**
 * Check for storage class of cell.
 *
 * @return true if cell has object storage class
 */
int lisp_is_object(lisp_cell_t * cell);

/**
 * Check for storage class of cell.
 *
 * @return true if cell has reference (imutable object) storage class
 */
int lisp_is_reference(lisp_cell_t * cell);

/**
 * Checks if cells has storage class of complex object, e.g. lisp_cons_t
 *
 * @return true if cell has complex object storage class
 */
int lisp_is_complex(lisp_cell_t * cell);

/**
 * Check if cell is cons
 *
 * @return true
 */
int lisp_is_cons(lisp_cell_t * cell);

struct lisp_cons_t * lisp_as_cons(lisp_cell_t * cell);

/**
 * get reference count for objects and complex objects.
 */
size_t lisp_get_ref_count(lisp_cell_t * cell);

int lisp_is_root_cell(lisp_cell_t * cell);

#endif
