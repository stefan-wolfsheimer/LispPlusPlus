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
/*
 * @todo create subdirectory for GC functions
 */
#ifndef __LISP_VM_H__
#define __LISP_VM_H__
#include "gc_color_map.h"
#include "typedefs.h"
#include <stdio.h>

/* dump modes for garbage collector dump function */
#define LISP_GC_DUMP_SILENT 0
#define LISP_GC_DUMP_HUMAN 1

struct lisp_cell_t;
struct lisp_gc_stat_t;
struct lisp_gc_iterator_t;
struct lisp_gc_stat_t;

typedef struct lisp_vm_t
{
  /* active conses */
  lisp_gc_collectible_list_t * cons_lists[LISP_GC_NUM_CLASSES];

  /* active objects */
  lisp_gc_collectible_list_t * object_lists[LISP_GC_NUM_CLASSES];

  /* cons memory */
  void ** cons_pages;
  void * current_cons_page;
  lisp_dl_list_t disposed_conses;
  lisp_dl_list_t disposed_objects;
  lisp_dl_list_t recycled_conses;
  size_t cons_page_size;
  size_t num_cons_pages;
  size_t cons_pos;

  /* number of cycles */
  size_t num_cycles;
  size_t num_steps;
} lisp_vm_t;

/**
 * Initialize lisp vm
 */
int lisp_init_vm(lisp_vm_t * vm);

/**
 * Free lisp vm
 */
int lisp_free_vm(lisp_vm_t * vm);

/*****************************************************************************
 lisp_cons_t memory management functions
 ****************************************************************************/
/**
 * Set the size of the cons pages.
 *
 * @return LISP_OK or LISP_INVALID
 */
int lisp_vm_gc_set_cons_page_size(lisp_vm_t * gc, size_t page_size);

size_t lisp_vm_gc_set_steps(lisp_vm_t * vm, size_t n);

/****************************************************************************
 lisp_vm_t allocator and garbage collector operation
 ****************************************************************************/
/**
 * Allocate a memory complex objects.
 */
void * lisp_vm_alloc_root_complex_object(lisp_vm_t * vm,
                                         lisp_type_id_t tid,
                                         size_t size);

/**
 * Execute a full garbage collector cycle
 */
int lisp_vm_gc_full_cycle(lisp_vm_t * vm);

/**
 * Unset all cars and cdrs of disposed conses.
 * Move disposed conses to recycled list.
 */
int lisp_vm_recycle_all_conses(lisp_vm_t * vm);

/**
 * recycle next cons.
 * @return LISP_NO_CHANGE of disposed list was empty LISP_OK otherwise
 */
int lisp_vm_recycle_next_cons(lisp_vm_t * vm);

/**
 * Unset all sub elements of disposed objects.
 * Move disposed object to recycled list.
 */
int lisp_vm_recycle_all_objects(lisp_vm_t * vm);

/**
 * recycle next cons.
 * @return LISP_NO_CHANGE of disposed list was empty LISP_OK otherwise
 */
int lisp_vm_recycle_next_object(lisp_vm_t * vm);

/**
 * Recycle all conses and objects
 */
int lisp_vm_recycle_all(lisp_vm_t * vm);

/**
 * Perform one garbage collector step and swap colors
 * if possible.
 * @return true if colors were swapped
 */
short int lisp_vm_gc_step(lisp_vm_t * vm);

/**
 * GC is swappable if there are no white or grey roots and
 * no grey objects or conses.
 *
 * @return true if GC is swappable
 */
short int lisp_vm_gc_swappable(lisp_vm_t * vm);

/**
 * Swap GC space if swappable
 */
short int lisp_vm_gc_swap(lisp_vm_t * vm);

/**
 * Perform one gargage collector step on the cons space.
 * @return true if conses are ready to be swapped.
 */
short int lisp_vm_gc_cons_step(lisp_vm_t * vm);
/**
 * @todo implement function
 */
short int lisp_vm_gc_object_step(lisp_vm_t * vm);

/****************************************************************************
 lisp_vm_t consistency checks, dump and statistics
 ****************************************************************************/
int lisp_vm_gc_check(lisp_vm_t * vm);

void lisp_vm_gc_dump(FILE * fp, lisp_vm_t * vm, int mode);

/**
 * Get garbage collector statistics.
 */
void lisp_vm_gc_get_stats(lisp_vm_t * vm,
                          struct lisp_gc_stat_t * stat);

#endif
