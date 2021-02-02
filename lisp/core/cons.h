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
#ifndef __LIPS_CONS_H__
#define __LIPS_CONS_H__
#include <stddef.h>
#include "cell.h"

struct lisp_type_t;
struct lisp_gc_collectible_list_t;
struct lisp_vm_t;

typedef struct lisp_cons_t
{
  struct lisp_gc_collectible_list_t * gc_list;
  size_t ref_count;

  lisp_cell_t car;
  lisp_cell_t cdr;
} lisp_cons_t;

/**
 * Create a cons.
 */
int lisp_make_cons(struct lisp_vm_t * vm,
                   struct lisp_cell_t * cell,
                   const struct lisp_cell_t * car,
                   const struct lisp_cell_t * cdr);

/******************************************************************************
 * car / cdr modification
 ******************************************************************************/
void lisp_cons_grey(lisp_cons_t * cons);

/**
 * Unset car cell
 */
int lisp_cons_unset_car(lisp_cons_t * cons);

/**
 * Unset cdr cell
 */
int lisp_cons_unset_cdr(lisp_cons_t * cons);

/**
 * Unset car and cdr cell
 */
int lisp_cons_unset_car_cdr(lisp_cons_t * cons);

/**
 * Set the car cell of the cons
 */
int lisp_cons_set_car(lisp_cons_t * cons,
                      lisp_cell_t * car);

/**
 * Set the cdr cell of the cons
 */
int lisp_cons_set_cdr(lisp_cons_t * cons,
                      lisp_cell_t * cdr);

int lisp_cons_set_car_cdr(lisp_cons_t * cons,
                          lisp_cell_t * car,
                          lisp_cell_t * cdr);


int lisp_init_cons_type(struct lisp_type_t * t);

#endif
