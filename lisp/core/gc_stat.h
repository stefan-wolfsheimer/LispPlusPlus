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
#ifndef __LISP_GC_STAT_H__
#define __LISP_GC_STAT_H__
#include <stdio.h>
#include "gc_color.h"

#define LISP_GC_STAT_NUM_FIELDS 36
#define LISP_GC_STAT_PRINT_COLOR 1

/**
 * Garbage collector statistics
 */
typedef struct lisp_gc_stat_t
{
  /**
   * Number of root objects (directly reachable)
   */
  size_t num_root;

  /**
   * Number of bulk objects (reachable from root)
   */
  size_t num_bulk;

  /**
   * Total number of objects.
   * num_reachable = num_root + num_bulk;
   */
  size_t num_reachable;

  /**
   * Number of allocated objects.
   * The (num_allocated - num_reachable - num_recyled) is the number
   * of unreachable objects.
   */
  size_t num_allocated;

  /**
   * Number of recycled conses.
   */
  size_t num_recycled;

  /**
   * Void Conses that have not been used yet.
   */
  size_t num_void;

  /**
   * Void Conses that have been marked as unreachable.
   */
  size_t num_disposed;

  /**
   * Number of full garbage collector cycles
   */
  size_t num_cycles;


  /**
   * Number of leave nodes of the reachibility graph.
   * That is the number of child objects that are not a complex object
   */
  size_t num_leaves;

  /**
   * Number of edges of the dependency graph.
   */
  size_t num_edges;

  /**
   * Number of cons pages
   */
  size_t num_cons_pages;

  size_t num_conses[LISP_GC_NUM_CLASSES];
  size_t num_objects[LISP_GC_NUM_CLASSES];
  /**
   * Error flags
   */
  int error_black_has_white_child;
  int error_cons_lists[LISP_GC_NUM_CLASSES];
  int error_object_lists[LISP_GC_NUM_CLASSES];
} lisp_gc_stat_t;


typedef struct lisp_gc_stat_field_t
{
  const char * name;
  const char * format;
  short int type;
  void * (*getter)(lisp_gc_stat_t *);
} lisp_gc_stat_field_t;

void lisp_init_gc_stat(lisp_gc_stat_t * stat);
int lisp_gc_stat_eq(lisp_gc_stat_t * stat1,
                    lisp_gc_stat_t * stat2);

void lisp_gc_stat_print(FILE * fp,
                        lisp_gc_stat_t * stat);

void lisp_gc_stat_print2(FILE * fp,
                         lisp_gc_stat_t * stat1,
                         lisp_gc_stat_t * stat2);

extern lisp_gc_stat_field_t lisp_gc_stat_field[LISP_GC_STAT_NUM_FIELDS];

#endif
