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
#ifndef __LIPS_TID_H__
#define __LIPS_TID_H__

#define LISP_NUM_TYPES 0x100

/**
 * storage classes
 *     << 5
 * 000 0x00 null types
 * 001 0x20 atom
 * 010 0x40 cow object requires constructor and destructor
 *                     but does not contain sub cells.
 *                     copy on write.
 * 011 0x60 object     managed object.
  *                    copied reference
 * 100 0x80 cons       object that can contains 2 sub cells
 * 101 0xa0 complex    object that can contains sub cells
 *                     e.g. array or hash table
 * 110 0xc0
 * 111 0xe0
 *
 */
#define LISP_STORAGE_NULL            0
#define LISP_STORAGE_ATOM            1
#define LISP_STORAGE_COW_OBJECT      2
#define LISP_STORAGE_OBJECT          3
#define LISP_STORAGE_CONS            4
#define LISP_STORAGE_COMPLEX         5

#define LISP_STORAGE_ID(__TID__) ((__TID__) >> 5)
#define LISP_TID_OFFSET(__SID__, __OFF__) (((__SID__) << 5) + (__OFF__))

#define LISP_TID_NIL      LISP_TID_OFFSET(LISP_STORAGE_NULL, 0)
#define LISP_TID_CONS     LISP_TID_OFFSET(LISP_STORAGE_CONS, 0)
#define LISP_TID_ARRAY    LISP_TID_OFFSET(LISP_STORAGE_COMPLEX, 1)

#define LISP_IS_CONS_TID(TID) ((TID) == LISP_TID_CONS)
#define LISP_IS_NIL_TID(TID)  ((TID) == LISP_TID_NIL)

struct lisp_cell_t;
struct lisp_cell_iterator_t;

typedef struct lisp_type_t
{
  /**
   * This function is called before the
   * memory is freed.
   */
  int (*lisp_destructor_ptr)(void * ptr);
  int (*lisp_cell_first_child_ptr)(struct lisp_cell_iterator_t * itr);
  int (*lisp_cell_child_iterator_is_valid_ptr)(struct lisp_cell_iterator_t * itr);
  int (*lisp_cell_next_child_ptr)(struct lisp_cell_iterator_t * itr);
} lisp_type_t;

/**
 * Ensure that static types are initialized.
 */
int lisp_load_static_types();

extern lisp_type_t lisp_static_types[LISP_NUM_TYPES];

#endif

