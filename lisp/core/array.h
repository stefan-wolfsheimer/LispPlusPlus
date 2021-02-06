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
#ifndef __LIPS_ARRAY_H__
#define __LIPS_ARRAY_H__
#include <stddef.h>
struct lisp_vm_t;
struct lisp_cell_t;
struct lisp_type_t;

typedef struct lisp_array_t
{
  struct lisp_cell_t * data;
  size_t size;
  size_t reserved; /*@todo implement reserved memory */
  size_t gc_pos;
} lisp_array_t;

/**
 * Create an array.
 */
int lisp_make_array(struct lisp_vm_t * vm,
                    struct lisp_cell_t * cell,
                    size_t n,
                    struct lisp_cell_t * value);

/******************************************************************************
 * modificators
 ******************************************************************************/
/**
 * Resize the array.
 */
int lisp_array_resize(struct lisp_array_t * array,
                      size_t n,
                      struct lisp_cell_t * value);

int lisp_array_append(struct lisp_array_t * array,
                      struct lisp_cell_t * value);

/**
 * Unset nth element of array.
 * \return LISP_RANGE_ERROR or LISP_OK
 */
int lisp_array_unset(struct lisp_array_t * array,
                     size_t n);

/**
 * Initialize static type.
 */
int lisp_init_array_type(struct lisp_type_t * t);

#endif
