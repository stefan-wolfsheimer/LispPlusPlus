#ifndef __LISP_VM_H__
#define __LISP_VM_H__
#include "gc.h"

struct lisp_cell_t;

typedef struct lisp_vm_t
{
  lisp_gc_t gc;
} lisp_vm_t;

/**
 * Initialize lisp vm
 */
int lisp_init_vm(lisp_vm_t * vm);

/**
 * Free lisp vm
 */
int lisp_free_vm(lisp_vm_t * vm);


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
#endif
