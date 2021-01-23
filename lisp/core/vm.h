#ifndef __LISP_VM_H__
#define __LISP_VM_H__
#include "gc_color_map.h"
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
  lisp_gc_color_map_t cons_color_map;

  /* active objects */
  lisp_gc_color_map_t object_color_map;

  /* cons memory */
  void ** cons_pages;
  void * current_cons_page;
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


/*****************************************************************************
 constructors
 ****************************************************************************/
/**
 * Create a cons as root.
 */
int lisp_make_cons(lisp_vm_t * vm,
                   struct lisp_cell_t * cell,
                   const struct lisp_cell_t * car,
                   const struct lisp_cell_t * cdr);

/**
 * Create an array as root.
 */
int lisp_make_array(lisp_vm_t * vm,
                    struct lisp_cell_t * cell,
                    size_t n);

/****************************************************************************
 lisp_vm_t garbage collector operation
 ****************************************************************************/
/**
 * Execute a full garbage collector cycle
 */
int lisp_vm_gc_full_cycle(lisp_vm_t * vm);

/**
 * @todo implement function
 */
void lisp_vm_gc_step(lisp_vm_t * vm);
/**
 * @todo implement function
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

void lisp_vm_gc_get_stats(lisp_vm_t * vm,
                          struct lisp_gc_stat_t * stat);
#endif
