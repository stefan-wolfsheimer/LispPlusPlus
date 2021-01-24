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
#ifndef __LISP_CELL_ITERATOR_H__
#define __LISP_CELL_ITERATOR_H__
#include <stddef.h>

struct lisp_type_t;
struct cell_t;

typedef struct lisp_cell_iterator_t
{
  struct lisp_cell_t * parent;
  struct lisp_cell_t * child;
  struct lisp_type_t * type;

  union
  {
    /* type specific field,
       i.e. index for arrays
     */
    size_t index;
  };
} lisp_cell_iterator_t;

/**
 * Initialize cell iterator.
 */
int lisp_first_child(struct lisp_cell_t  * cell,
                     lisp_cell_iterator_t * itr);


/**
 * @return true if iterator is valid
 */
int lisp_cell_iterator_is_valid(lisp_cell_iterator_t * itr);

/**
 * Iterate to the next child
 */
int lisp_cell_next_child(lisp_cell_iterator_t * itr);


#endif
