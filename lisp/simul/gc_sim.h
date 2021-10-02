#ifndef __GC_SIM_H__
#define __GC_SIM_H__
#include <stddef.h>
#include <limits.h>
#include <lisp/util/hash_table.h>
#include <lisp/core/vm.h>
#include <lisp/core/cell.h>

#define LISP_MAX_SIZE_T ((size_t)-1)

typedef struct lisp_gc_sim_node_t
{
  int is_root;
  size_t node_index;
  lisp_cell_t cell;
} lisp_gc_sim_node_t;

typedef struct lisp_gc_sim_leaf_t
{
  lisp_gc_sim_node_t * node;
  size_t index;
} lisp_gc_sim_leaf_t;

typedef struct lisp_gc_sim_edge_t
{
  lisp_gc_sim_node_t * parent;
  lisp_gc_sim_node_t * child;
  size_t index;
} lisp_gc_sim_edge_t;

typedef struct lisp_gc_sim_t
{
  lisp_vm_t vm;
  hash_table_t reachable;

  lisp_cell_t ** root;
  size_t num_root;

  lisp_cell_t * cell;
  size_t num_cells;

  lisp_gc_sim_node_t * nodes;
  size_t num_nodes;

  lisp_gc_sim_leaf_t * leaves;
  size_t num_leaves;

  lisp_gc_sim_edge_t * edges;
  size_t num_edges;

} lisp_gc_sim_t;

int lisp_init_gc_sim(lisp_gc_sim_t * sim);
int lisp_free_gc_sim(lisp_gc_sim_t * sim);

lisp_gc_sim_node_t * lisp_sim_find_node(lisp_gc_sim_t * sim,
                                        const lisp_cell_t * cell);

/**
 * Create a new root cell with n_children increment append
 * the array sim->root and increate sim->num_root by 1.
 * If n_children = 2, a cons is created otherwise an array.
 *
 * Refresh the graph after the new element has been added.
 *
 */
lisp_cell_t * lisp_sim_add_root(lisp_gc_sim_t * sim,
                                size_t n_children);

int lisp_sim_add_bulk(lisp_gc_sim_t * sim,
                      lisp_cell_t * parent,
                      size_t index,
                      lisp_cell_t  * child,
                      size_t n_children);

int lisp_sim_add_edge(lisp_gc_sim_t * sim,
                      lisp_cell_t * parent,
                      size_t index,
                      lisp_cell_t  * child);

int lisp_sim_remove_edge(lisp_gc_sim_t * sim,
                         lisp_cell_t * parent,
                         size_t index);

int lisp_sim_unroot(lisp_gc_sim_t * sim, lisp_cell_t  * child);

#endif
