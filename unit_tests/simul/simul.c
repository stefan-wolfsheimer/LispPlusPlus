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
of the authors and should not be interpreted as representing of official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/
#include <lisp/util/unit_test.h>
#include <lisp/util/xmalloc.h>
#include <lisp/simul/gc_sim.h>
#include <lisp/core/cons.h>
#include <lisp/core/array.h>
#include <unit_tests/common.h>

static int check_gc_sim_nodes(lisp_gc_sim_t * sim)
{
  size_t i;
  lisp_gc_sim_node_t * n;
  for(i = 0; i < sim->num_nodes; i++)
  {
    n = lisp_sim_find_node(sim, &sim->nodes[i].cell);
    if(n  != sim->nodes + i)
    {
      return 0;
    }
  }
  return 1;
}

static int check_gc_leaf_node(lisp_cell_t * cell, size_t index)
{
  lisp_cons_t * cons;
  lisp_array_t * array;
  if(lisp_is_cons(cell))
  {
    cons = lisp_as_cons(cell);
    if(index == 0)
    {
      return lisp_is_nil(lisp_cons_get_car(cons)) ? 1 : 0;
    }
    else if(index == 1)
    {
      return lisp_is_nil(lisp_cons_get_cdr(cons)) ? 1 : 0;
    }
    else
    {
      return 0;
    }
  }
  else if(lisp_is_array(cell))
  {
    array = lisp_as_array(cell);
    if(index < lisp_array_size(array))
    {
      return lisp_is_nil(lisp_array_get(array, index)) ? 1 : 0;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

static int check_gc_sim_leaves(lisp_gc_sim_t * sim)
{
  size_t i;
  for(i = 0; i < sim->num_leaves; i++)
  {
    if(!(sim->leaves[i].node >= sim->nodes &&
         sim->leaves[i].node <  (sim->nodes + sim->num_nodes)))
    {
      return 0;
    }
    if(!check_gc_leaf_node(&sim->leaves[i].node->cell, sim->leaves[i].index))
    {
      return 0;
    }
  }
  return 1;
}

static int check_gc_edge(lisp_gc_sim_edge_t * edge)
{
  lisp_cell_t * cell;
  const lisp_cell_t * child;
  lisp_cons_t * cons;
  lisp_array_t * array;
  cell = &edge->parent->cell;
  child = NULL;
  if(lisp_is_cons(cell))
  {
    cons = lisp_as_cons(cell);
    if(edge->index == 0)
    {
      child = lisp_cons_get_car(cons);
    }
    else if(edge->index == 1)
    {
      child = lisp_cons_get_cdr(cons);
    }
    else
    {
      return 0;
    }
  }
  else if(lisp_is_array(cell))
  {
    array = lisp_as_array(cell);
    if(edge->index < lisp_array_size(array))
    {
      child = lisp_array_get(array, edge->index);
    }
    else
    {
      return 0;
    }
  }
  if(child)
  {
    if(lisp_is_cons(child))
    {
      return ( lisp_as_cons(child) == lisp_as_cons(&edge->child->cell));
    }
    else if(lisp_is_array(child))
    {
      return ( lisp_as_array(child) == lisp_as_array(&edge->child->cell));
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

static int check_gc_sim_edges(lisp_gc_sim_t * sim)
{
  size_t i;
  for(i = 0; i < sim->num_edges; i++)
  {
    if(!(sim->edges[i].parent >= sim->nodes &&
         sim->edges[i].parent <  (sim->nodes + sim->num_nodes)))
    {
      return 0;
    }
    if(!(sim->edges[i].child >= sim->nodes &&
         sim->edges[i].child <  (sim->nodes + sim->num_nodes)))
    {
      return 0;
    }
    if(!check_gc_edge(&sim->edges[i]))
    {
      return 0;
    }
  }
  return 1;
}

static int check_gc_sim(unit_test_t * tst,
                        lisp_gc_sim_t * sim,
                        lisp_gc_stat_t * ref_stat)
{
  if(!CHECK_EQ_U(tst, sim->num_root, ref_stat->num_root) ||
     !CHECK_EQ_U(tst, sim->num_leaves, ref_stat->num_leaves) ||
     !CHECK_EQ_U(tst, sim->num_nodes, ref_stat->num_root + ref_stat->num_bulk) ||
     !CHECK_EQ_U(tst, sim->num_edges, ref_stat->num_edges) ||
     !CHECK(tst, check_gc_sim_nodes(sim)) ||
     !CHECK(tst, check_gc_sim_leaves(sim)) ||
     !CHECK(tst, check_gc_sim_edges(sim)))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

static void test_sim_cycle(unit_test_t * tst)
{
  lisp_gc_sim_t sim;
  lisp_gc_stat_t ref_stat;
  lisp_cell_t * root_node0;
  lisp_cell_t * root_node3;
  lisp_cell_t node0;
  lisp_cell_t node1;
  lisp_cell_t node2;
  lisp_cell_t node4;
  memcheck_begin();
  lisp_init_gc_stat(&ref_stat);
  ASSERT_LISP_OK(tst, lisp_init_gc_sim(&sim));
  ASSERT_LISP_OK(tst, lisp_vm_gc_set_cons_page_size(&sim.vm, 4));
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);


  /**
   * nodes          leaves
   * 0 (R): cons    0: (0, 0) -> NULL
   *                1: (0, 1) -> NULL
   *
   * edges
   */
  root_node0 = lisp_sim_add_root(&sim, 2u);
  node0 = *root_node0;
  ASSERT_NEQ_PTR(tst, root_node0, NULL);
  ASSERT(tst, lisp_is_root_cell(root_node0));
  ref_stat.num_root = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_reachable = 1;
  ref_stat.num_allocated = 1;
  ref_stat.num_void = 3;
  ref_stat.num_cons_pages = 1;
  ref_stat.num_conses[LISP_GC_WHITE_ROOT] = 1;
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);

  /**
   * nodes          leaves
   * 0 (R): cons    0: (0, 1) -> NULL
   * 1: array
   *
   * edges:
   * 0: (0, 0) -> 1
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_add_bulk(&sim, root_node0, 0u, &node1, 0u));
  ref_stat.num_bulk = 1;
  ref_stat.num_edges = 1;
  ref_stat.num_leaves = 1;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_objects[LISP_GC_GREY] = 1;
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);

  /**
   * nodes            leaves
   * 0 (R): cons

   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_add_bulk(&sim, root_node0, 1u, &node2, 3u));
  ref_stat.num_bulk = 2;
  ref_stat.num_edges = 2;
  ref_stat.num_leaves = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_objects[LISP_GC_GREY] = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * nodes            leaves
   * 0 (R): cons

   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)  0: (3, 0) -> NULL
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   *
   */

  root_node3 = lisp_sim_add_root(&sim, 1u);
  ASSERT_NEQ_PTR(tst, root_node3, NULL);
  ASSERT(tst, lisp_is_root_cell(root_node3));
  ref_stat.num_root = 2;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 4;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * nodes            leaves
   * 0 (R): cons

   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons          3: (4, 0) -> NULL
   *                  4: (4, 1) -> NULL
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_add_bulk(&sim, root_node3, 0, &node4, 2u));
  ref_stat.num_bulk = 3;
  ref_stat.num_reachable = 5;
  ref_stat.num_leaves = 5;
  ref_stat.num_edges = 3;
  ref_stat.num_allocated = 5;
  ref_stat.num_void = 2;
  ref_stat.num_conses[LISP_GC_GREY] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * Add edge (4, 1) -> 0
   * --------------------
   *
   * nodes            leaves
   * 0 (R): cons

   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons          3: (4, 0) -> NULL
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   *
   */
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT_LISP_OK(tst, lisp_sim_add_edge(&sim, &node4, 1, root_node0));
  ref_stat.num_leaves = 4;
  ref_stat.num_edges = 4;
  ref_stat.num_conses[LISP_GC_WHITE_ROOT] = 0;
  ref_stat.num_conses[LISP_GC_GREY_ROOT] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * Add edge (4, 0) -> 4
   * --------------------
   *
   * nodes            leaves
   * 0 (R): cons

   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_add_edge(&sim, &node4, 0, &node4));
  ref_stat.num_leaves = 3;
  ref_stat.num_edges = 5;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * unroot node0
   * ------------
   *
   * 0: cons
   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   *
   */
  ASSERT(tst, lisp_is_root_cell(root_node0));
  ASSERT_LISP_OK(tst, lisp_sim_unroot(&sim, root_node0));
  ref_stat.num_root = 1;
  ref_stat.num_bulk = 4;
  ref_stat.num_conses[LISP_GC_GREY] = 2;
  ref_stat.num_conses[LISP_GC_GREY_ROOT] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * remove edge (0, 0) -> 1
   * -----------------------
   *
   * 0: cons          0: (0, 0) -> NULL
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons
   *
   * edges:
   * 0: (0, 1) -> 2
   * 1: (3, 0) -> 4
   * 2: (4, 1) -> 0
   * 3: (4, 0) -> 4
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_remove_edge(&sim, &node0, 0 ));
  ref_stat.num_bulk = 3;       /* -1 */
  ref_stat.num_reachable = 4;  /* -1 */
  ref_stat.num_leaves = 4;     /* +1 */
  ref_stat.num_edges = 4;      /* -1 */
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * add edge (0, 0) -> 2
   * -----------------------
   *
   * 0: cons
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons
   *
   * edges:
   * 0: (0, 1) -> 2
   * 1: (3, 0) -> 4
   * 2: (4, 1) -> 0
   * 3: (4, 0) -> 4
   * 4: (0, 0) -> 2
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_add_edge(&sim, &node0, 0, &node2));
  ref_stat.num_leaves = 3; /* -1 */
  ref_stat.num_edges = 5;  /* +1 */
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * remove edge (0, 1) -> 2
   * -----------------------
   *
   * 0: cons          0: (0, 1) -> NULL
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons
   *
   * edges:
   * 0: (3, 0) -> 4
   * 1: (4, 1) -> 0
   * 2: (4, 0) -> 4
   * 3: (0, 0) -> 2
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_remove_edge(&sim, &node0, 1));
  ref_stat.num_leaves = 4; /* +1 */
  ref_stat.num_edges = 4;  /* -1 */
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * remove edge (4, 1) -> 0
   * -----------------------
   * 0: cons unreachable
   * 2: array unreachabel
   *
   * 0: cons          0: (0, 1) -> NULL
   * 2: array(0,1,2)  0: (2, 0) -> NULL
   *                  1: (2, 1) -> NULL
   *                  2: (2, 2) -> NULL
   * 3 (R): array(0)
   * 4: cons
   *
   * edges:
   * 0: (3, 0) -> 4
   * 1: (4, 0) -> 4
   *
   */
  ASSERT_LISP_OK(tst, lisp_sim_remove_edge(&sim, &node4, 1));
  ref_stat.num_bulk = 1;      /* -2 */
  ref_stat.num_reachable = 2; /* -2 */
  ref_stat.num_leaves = 1;    /* -4 + 1 */
  ref_stat.num_edges = 2;     /* -2 */
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  /**
   * unroot node3
   */
  ASSERT(tst, lisp_is_root_cell(root_node3));
  ASSERT_LISP_OK(tst, lisp_sim_unroot(&sim, root_node3));
  root_node3 = NULL;
  ref_stat.num_root = 0;
  ref_stat.num_bulk = 0;
  ref_stat.num_reachable = 0;
  ref_stat.num_leaves = 0;
  ref_stat.num_edges = 0;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_GREY] = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &sim.vm, &ref_stat);
  ASSERT(tst, check_gc_sim(tst, &sim, &ref_stat));

  ASSERT_LISP_OK(tst, lisp_free_gc_sim(&sim));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_simul(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "simul");
  TEST(suite, test_sim_cycle);
}


