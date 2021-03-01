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
#ifndef __LISP_CELL_H__
#define __LISP_CELL_H__
#include <stddef.h>
#include "gc_color.h"
#include "typedefs.h"

struct hash_table_t;

typedef struct lisp_cell_t
{
  lisp_type_id_t type_id;
  union
  {
    void * obj;
  } data;
} lisp_cell_t;

extern lisp_cell_t lisp_nil;

int lisp_make_nil(lisp_cell_t * cell);

/**
 * Check for storage class of cell.
 *
 * @return true if cell has null storage class
 */
int lisp_is_null(const lisp_cell_t * cell);

/**
 * Return if cell is NIL
 */
int lisp_is_nil(const lisp_cell_t * cell);

/**
 * Check for storage class of cell.
 *
 * @return true if cell has atomic storage class
 */
int lisp_is_atomic(const lisp_cell_t * cell);

/**
 * Check for storage class of cell.
 *
 * @return true if cell has cow object storage class
 */
int lisp_is_cow_object(const lisp_cell_t * cell);

/**
 * Check for storage class of cell.
 *
 * @return true if cell has reference (imutable object) storage class
 */
int lisp_is_object(const lisp_cell_t * cell);

/**
 * Check if cell is cons
 *
 * @return true
 */
int lisp_is_cons(const lisp_cell_t * cell);

/**
 * Checks if cell has storage class of complex object, e.g. array
 *
 * @return true if cell has complex object storage class
 */
int lisp_is_complex(const lisp_cell_t * cell);


struct lisp_complex_object_t * lisp_as_complex_object(const lisp_cell_t * cell);
struct lisp_cons_t * lisp_as_cons(const lisp_cell_t * cell);

int lisp_is_array(const lisp_cell_t * cell);
struct lisp_array_t * lisp_as_array(const lisp_cell_t * cell);

/**
 * get reference count for objects,
 * conses and complex objects.
 */
size_t lisp_get_ref_count(const lisp_cell_t * cell);

/**
 * Returns true if cell is atomic, object or reference.
 * If cell is complex or cons, returns if object is in root set.
 */
int lisp_is_root_cell(const lisp_cell_t * cell);

/**
 * Return color of cell object if object is complex or cons.
 * NO_COLOR otherwise.
 */
lisp_gc_color_t lisp_get_cell_color(const lisp_cell_t * cell);

/**
 * Reset to nil.
 */
int lisp_unset(lisp_cell_t * cell);

/*****************************************************************************
 * internal functions
 *****************************************************************************/
/**
 * Init child cell of cons or complex object.
 *
 * \param target pointer to (uninitialized) cell
 * \param source
 * \return LISP_OK if successful
 */
int lisp_init_child_cell(lisp_cell_t * target, const lisp_cell_t * source);

/**
 * Unset child cell and set new value
 *
 * \param target pointer to initialized cell
 * \param source
 * \return LISP_OK if successful
 */
int lisp_set_child_cell(lisp_cell_t * target, const lisp_cell_t * source);

/**
 * Unset child cell of cons or complex object.
 * \return LISP_OK
 */
int lisp_unset_child_cell(lisp_cell_t * target);

#endif
