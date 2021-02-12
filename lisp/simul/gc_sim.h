#ifndef __GC_SIM_H__
#define __GC_SIM_H__
#include <stddef.h>
#include <lisp/core/vm.h>
#include <lisp/core/cell.h>


typedef struct lisp_gc_sim_t
{
  lisp_vm_t vm;
  lisp_cell_t * root;
  size_t n_root;
} lisp_gc_sim_t;

int lisp_init_gc_sim(lisp_gc_sim_t * sim);
int lisp_free_gc_sim(lisp_gc_sim_t * sim);
  
/**
 * Store the i th root in cell.
 * i index of the root in [0, stat->num_root )
 */
void lisp_sim_get_root(lisp_vm_t * vm,
                       struct lisp_cell_t * cell,
                       size_t i);

/**
 * Store the i th root in cell.
 * i index of the root in [0, stat->num_bulk )
 */
void lisp_sim_get_bulk(lisp_vm_t * vm,
                       struct lisp_cell_t * cell,
                       size_t i);

/**
 * Store the i th root in cell.
 * i index of the root in [0, stat->num_reachable )
 */
void lisp_sim_get_node(lisp_vm_t * vm,
                       struct lisp_cell_t * cell,
                       size_t i);

int lisp_sim_add_root(lisp_vm_t * vm,
                      size_t n);

int lisp_sim_add_bulk(lisp_vm_t * vm,
                      size_t free_child_index,
                      size_t n);

int lisp_sim_add_edge(lisp_vm_t * vm,
                      size_t free_child_index,
                      size_t node_index);

int lisp_sim_remove_edge(struct lisp_vm_t * vm,
                         size_t index);

int lisp_sim_unroot(struct lisp_vm_t * vm,
                    size_t index);

#endif
