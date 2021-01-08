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
#ifndef __LISP_GC_ITERATOR_H__
#define __LISP_GC_ITERATOR_H__
#include "cell.h"
#include <lisp/util/hash_table.h>
struct lisp_dl_item_t;
struct lisp_gc_t;

/**
 * Iterator for all complex objects in garbage collector.
 * (conses and other objects such as arrays)
 */
typedef struct lisp_gc_iterator_t
{
  lisp_cell_t cell;
  int current_index;
  struct lisp_dl_item_t * current_item;
} lisp_gc_iterator_t;

/**
 * Iterator over all reachable conses / objects
 */
typedef struct lisp_gc_reachable_iterator_t
{
  hash_table_t todo;
  hash_table_t root;
  hash_table_entry_t * entry;
} lisp_gc_reachable_iterator_t;

/**
 * Initialize GC iterator.
 */
void lisp_gc_first(struct lisp_gc_t * gc, lisp_gc_iterator_t * itr);

/**
 * @return true if iterator is valid
 */
int lisp_gc_iterator_is_valid(lisp_gc_iterator_t * itr);

/**
 * Iterate to the next object in garbage collector
 */
int lisp_gc_next(struct lisp_gc_t * gc, lisp_gc_iterator_t * itr);

/**
 * Initialize GC reachable iterator.
 */
int lisp_init_gc_reachable_iterator(lisp_gc_reachable_iterator_t * itr);
void lisp_free_gc_reachable_iterator(lisp_gc_reachable_iterator_t * itr);

void lisp_gc_reachable_first(struct lisp_gc_t * gc,
                             lisp_gc_reachable_iterator_t * itr);
int lisp_gc_reachable_iterator_is_valid(lisp_gc_reachable_iterator_t * itr);
int lisp_gc_reachable_next(struct lisp_gc_t * gc,
                           lisp_gc_reachable_iterator_t * itr);
#endif
