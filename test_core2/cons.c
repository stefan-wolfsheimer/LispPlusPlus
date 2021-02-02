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
#include <lisp/util/unit_test.h>
#include <lisp/util/xmalloc.h>
#include <lisp/core/vm.h>
#include <lisp/core/cons.h>
#include <lisp/core/error.h>
#include "common.h"

static void test_make_cons(unit_test_t * tst)
{
  lisp_vm_t vm;
  lisp_gc_stat_t ref_stat;
  lisp_cell_t cell[6];

  memcheck_begin();
  lisp_init_gc_stat(&ref_stat);
  ASSERT_LISP_OK(tst, lisp_init_vm(&vm));
  lisp_vm_gc_set_steps(&vm, 0);
  ASSERT_LISP_OK(tst, lisp_vm_gc_set_cons_page_size(&vm, 4));
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[0] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[0], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[0]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[0]));
  ref_stat.num_root = 1;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_reachable = 1;
  ref_stat.num_allocated = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_void = 3;
  ref_stat.num_cons_pages = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[1] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[1], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[1]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[1]));
  ref_stat.num_root = 2;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_leaves = 4;
  ref_stat.num_void = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[2] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[2], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[2]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[2]));
  ref_stat.num_root = 3;
  ref_stat.num_white_root_conses = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_leaves = 6;
  ref_stat.num_void = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[3] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[3], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[3]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[3]));
  ref_stat.num_root = 4;
  ref_stat.num_white_root_conses = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 8;
  ref_stat.num_void = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[4] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[4], NULL, NULL));
  ASSERT(tst, lisp_is_root_cell(&cell[4]));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[4]), LISP_GC_WHITE);
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 5;
  ref_stat.num_white_root_conses = 5;
  ref_stat.num_reachable = 5;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 10;
  ref_stat.num_void = 3;
  ref_stat.num_cons_pages = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cons objects unqiue */
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), NULL);
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), lisp_as_cons(&cell[1]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), lisp_as_cons(&cell[2]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), lisp_as_cons(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), lisp_as_cons(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[1]), lisp_as_cons(&cell[2]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[1]), lisp_as_cons(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[1]), lisp_as_cons(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[2]), lisp_as_cons(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[2]), lisp_as_cons(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[3]), lisp_as_cons(&cell[4]));

  /* unset cell[0] cell[1] cell[2] */
  ASSERT_LISP_OK(tst, lisp_unset(&cell[0]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[1]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[2]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[0]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[1]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[2]));

  ref_stat.num_root = 2;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_grey_conses = 3;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 4;
  ref_stat.num_void = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cycle GC */
  ASSERT_LISP_OK(tst, lisp_vm_gc_full_cycle(&vm));
  ref_stat.num_cycles = 1;
  ref_stat.num_root = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_leaves = 4;
  ref_stat.num_void = 3;
  ref_stat.num_disposed = 3;
  ref_stat.num_grey_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);
  ASSERT_LISP_OK(tst, lisp_vm_recycle_all_conses(&vm));
  ref_stat.num_disposed = 0;
  ref_stat.num_recycled = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);

  /* make cons cell[0] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[0], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[0]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[0]));
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ASSERT_LISP_OK(tst, lisp_vm_recycle_all_conses(&vm));

  ref_stat.num_root = 3;
  ref_stat.num_white_root_conses = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_leaves = 6;
  ref_stat.num_recycled = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[1] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[1], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[1]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[1]));
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 4;
  ref_stat.num_white_root_conses = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 8;
  ref_stat.num_recycled = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[2] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[2], NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[1]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[2]));
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 5;
  ref_stat.num_white_root_conses = 5;
  ref_stat.num_reachable = 5;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 10;
  ref_stat.num_recycled = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cell[5] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[5], NULL, NULL));
  ASSERT_EQ_I(tst, vm.cons_pos, 2u);
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[5]), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell[5]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), NULL);
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[0]), lisp_as_cons(&cell[1]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[1]), lisp_as_cons(&cell[2]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[2]), lisp_as_cons(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[3]), lisp_as_cons(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell[4]), lisp_as_cons(&cell[5]));
  ref_stat.num_root = 6;
  ref_stat.num_white_root_conses = 6;
  ref_stat.num_reachable = 6;
  ref_stat.num_allocated = 6;
  ref_stat.num_leaves = 12;
  ref_stat.num_recycled = 0;
  ref_stat.num_void = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_cons_and_recyle(unit_test_t * tst)
{
  lisp_vm_t vm;
  lisp_gc_stat_t ref_stat;
  lisp_cell_t cell[6];

  memcheck_begin();
  lisp_init_gc_stat(&ref_stat);
  ASSERT_LISP_OK(tst, lisp_init_vm(&vm));
  lisp_vm_gc_set_steps(&vm, 0);
  ASSERT_LISP_OK(tst, lisp_vm_gc_set_cons_page_size(&vm, 4));
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell[0] - cell[4] */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[0], NULL, NULL));
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[1], NULL, NULL));
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[2], NULL, NULL));
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[3], NULL, NULL));
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell[4], NULL, NULL));

  /* unset cell[0] cell[1] cell[2] */
  ASSERT_LISP_OK(tst, lisp_unset(&cell[0]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[1]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[2]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[0]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[1]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[2]));

  ref_stat.num_root = 2;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_grey_conses = 3;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 4;
  ref_stat.num_recycled = 0;
  ref_stat.num_void = 3;
  ref_stat.num_cons_pages = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ref_stat.num_white_root_conses = 0;
  ref_stat.num_black_root_conses = 2;
  ref_stat.num_grey_conses = 0;
  ref_stat.num_grey_root_conses = 0;
  ref_stat.num_black_conses = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* first swap */
  ASSERT(tst, lisp_vm_gc_swap(&vm));
  ref_stat.num_cycles = 1;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_black_root_conses = 0;
  ref_stat.num_white_conses = 3;
  ref_stat.num_black_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_white_root_conses = 0;
  ref_stat.num_black_root_conses = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* second swap */
  ASSERT(tst, lisp_vm_gc_swap(&vm));
  ref_stat.num_cycles = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_root = 2;
  ref_stat.num_disposed = 3;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_black_root_conses = 0;
  ref_stat.num_white_conses = 0;
  ref_stat.num_black_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_cons_of_conses(unit_test_t * tst)
{
  lisp_gc_stat_t ref_stat;
  lisp_vm_t vm;
  lisp_cell_t root1;
  lisp_cell_t child1;
  lisp_cell_t child2;

  memcheck_begin();
  lisp_init_gc_stat(&ref_stat);
  ASSERT_LISP_OK(tst, lisp_init_vm(&vm));
  lisp_vm_gc_set_steps(&vm, 0);
  ASSERT_LISP_OK(tst, lisp_vm_gc_set_cons_page_size(&vm, 4));

  /* create first child
     child1 = (nil . nil)
   */
  ASSERT_LISP_OK(tst,
                 lisp_make_cons(&vm,
                                &child1,
                                &lisp_nil,
                                &lisp_nil));
  ASSERT(tst, lisp_is_cons(&child1));
  ASSERT(tst, lisp_is_root_cell(&child1));
  ASSERT(tst, lisp_get_cell_color(&child1) == LISP_GC_WHITE);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 1u);

  ref_stat.num_root = 1;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_allocated = 1;
  ref_stat.num_reachable = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_cons_pages = 1;
  ref_stat.num_void = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create second child
     child1 = (nil . nil)
     child2 = (nil . nil)
   */
  ASSERT_LISP_OK(tst,
                 lisp_make_cons(&vm,
                                &child2,
                                &lisp_nil,
                                &lisp_nil));
  ASSERT(tst, lisp_is_cons(&child2));
  ASSERT(tst, lisp_is_root_cell(&child2));
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_WHITE);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child2), 1u);

  ref_stat.num_root = 2;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_leaves = 4;
  ref_stat.num_void = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create cons(child1, child2)
     child1 = (nil . nil)
     child2 = (nil . nil)
     root = ( (nil . nil) . (nil . nil) )
   */
  ASSERT_LISP_OK(tst,
                 lisp_make_cons(&vm,
                                &root1,
                                &child1,
                                &child2));
  ASSERT(tst, lisp_is_cons(&root1));
  ASSERT(tst, lisp_is_root_cell(&root1));
  ASSERT(tst, lisp_get_cell_color(&root1) == LISP_GC_WHITE);
  ASSERT(tst, lisp_get_cell_color(&child1) == LISP_GC_GREY);
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_GREY);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&root1), 1u);
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_GREY);

  ref_stat.num_root = 3;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_grey_root_conses = 2;
  ref_stat.num_allocated = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_edges = 2;
  ref_stat.num_void = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset child1, child2
     root = ( (nil . nil) . (nil . nil) )
  */
  ASSERT_LISP_OK(tst, lisp_unset(&child1));
  ASSERT_LISP_OK(tst, lisp_unset(&child2));
  ASSERT(tst, lisp_is_nil(&child1));
  ASSERT(tst, lisp_is_nil(&child2));
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 0u);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child2), 0u);
  ASSERT_FALSE(tst, lisp_is_root_cell(&child1));
  ASSERT_FALSE(tst, lisp_is_root_cell(&child2));

  ref_stat.num_root = 1;
  ref_stat.num_grey_root_conses = 0;
  ref_stat.num_grey_conses = 2;
  ref_stat.num_bulk = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_white_root_conses = 0;
  ref_stat.num_black_root_conses = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* second gc step */
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));

  ref_stat.num_grey_conses = 1;
  ref_stat.num_black_conses = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* third gc step */
  ASSERT(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_swap(&vm));

  ref_stat.num_cycles = 1;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_black_root_conses = 0;
  ref_stat.num_grey_conses = 0;
  ref_stat.num_white_conses = 2;
  ref_stat.num_black_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset car / cdr
     root = ( nil . nil )
  */
  ASSERT_LISP_OK(tst, lisp_cons_unset_car(lisp_as_cons(&root1)));
  ASSERT_LISP_OK(tst, lisp_cons_unset_cdr(lisp_as_cons(&root1)));

  ref_stat.num_bulk = 0;
  ref_stat.num_reachable = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_edges = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* 4th gc step */
  ASSERT(tst, lisp_vm_gc_cons_step(&vm));
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_swap(&vm));

  ref_stat.num_allocated = 1;
  ref_stat.num_disposed = 2;
  ref_stat.num_cycles = 2;
  ref_stat.num_white_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* recycle conses */
  ASSERT_LISP_OK(tst, lisp_vm_recycle_next_cons(&vm));
  ASSERT_LISP_OK(tst, lisp_vm_recycle_next_cons(&vm));
  ASSERT_EQ_I(tst, lisp_vm_recycle_next_cons(&vm), LISP_NO_CHANGE);

  ref_stat.num_recycled = 2;
  ref_stat.num_disposed = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create first child
     root = ( nil . nil )
     child1 = ( nil . nil )
  */
  ASSERT_LISP_OK(tst,
                 lisp_make_cons(&vm,
                                &child1,
                                &lisp_nil,
                                &lisp_nil));
  ASSERT(tst, lisp_is_cons(&child1));
  ASSERT(tst, lisp_is_root_cell(&child1));
  ASSERT(tst, lisp_get_cell_color(&child1) == LISP_GC_WHITE);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 1u);

  ref_stat.num_root = 2;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_leaves = 4;
  ref_stat.num_recycled = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create second child
     root = ( nil . nil )
     child1 = ( nil . nil )
     child2 = ( nil . nil )
  */
  ASSERT_LISP_OK(tst,
                 lisp_make_cons(&vm,
                                &child2,
                                &lisp_nil,
                                &lisp_nil));
  ASSERT(tst, lisp_is_cons(&child2));
  ASSERT(tst, lisp_is_root_cell(&child2));
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_WHITE);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child2), 1u);

  ref_stat.num_root = 3;
  ref_stat.num_white_root_conses = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_leaves = 6;
  ref_stat.num_recycled = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* set car, cdr
     root = ( ( nil . nil ) . ( nil . nil ) )
     child1 = ( nil . nil )
     child2 = ( nil . nil )
  */
  ASSERT_LISP_OK(tst, lisp_cons_set_car_cdr(lisp_as_cons(&root1),
                                            &child1,
                                            &child2));

  ref_stat.num_leaves = 4;
  ref_stat.num_edges = 2;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_grey_root_conses = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset child1, child2
     root = ( ( nil . nil ) . ( nil . nil ) )
     child1 = nil
     child2 = nil
  */
  ASSERT_LISP_OK(tst, lisp_unset(&child1));
  ASSERT_LISP_OK(tst, lisp_unset(&child2));
  ASSERT(tst, lisp_is_nil(&child1));
  ASSERT(tst, lisp_is_nil(&child2));

  ref_stat.num_root = 1;
  ref_stat.num_grey_root_conses = 0;
  ref_stat.num_grey_conses = 2;
  ref_stat.num_bulk = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create first child
     root = ( ( nil . nil ) . ( nil . nil ) )
     child1 = ( nil . nil )
  */
  ASSERT_LISP_OK(tst,
                 lisp_make_cons(&vm,
                                &child1,
                                &lisp_nil,
                                &lisp_nil));
  ASSERT(tst, lisp_is_cons(&child1));
  ASSERT(tst, lisp_is_root_cell(&child1));
  ASSERT(tst, lisp_get_cell_color(&child1) == LISP_GC_WHITE);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 1u);

  ref_stat.num_root = 2;
  ref_stat.num_white_root_conses = 2;
  ref_stat.num_allocated = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_leaves = 6;
  ref_stat.num_recycled = 0;
  ref_stat.num_void = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* set car cdr
     child1 = ( nil . nil )
     root = ( child1 . child1 )
  */
  ASSERT_LISP_OK(tst, lisp_cons_set_car_cdr(lisp_as_cons(&root1),
                                            &child1,
                                            &child1));
  ref_stat.num_bulk = 0;
  ref_stat.num_reachable = 2;
  ref_stat.num_leaves = 2;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_grey_root_conses = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset child1
     child1 = nil
     root = ( #1 . #1 )
     #1 = (nil, nil)
  */
  ASSERT_LISP_OK(tst, lisp_unset(&child1));
  ASSERT(tst, lisp_is_nil(&child1));

  ref_stat.num_bulk = 1;
  ref_stat.num_root = 1;
  ref_stat.num_reachable = 2;
  ref_stat.num_leaves = 2;
  ref_stat.num_grey_conses = 3;
  ref_stat.num_grey_root_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* set car(car root) = root
     set cdr(cdr root) = root
     child1 = nil
     root = ( #1 . #1 )
     #1 = (root, root)
  */
  ASSERT_LISP_OK(tst,
                 lisp_cons_set_car(lisp_as_cons(&lisp_as_cons(&root1)->car),
                                   &root1));
  ASSERT_LISP_OK(tst,
                 lisp_cons_set_cdr(lisp_as_cons(&lisp_as_cons(&root1)->cdr),
                                   &root1));
  ASSERT_EQ_PTR(tst,
                lisp_as_cons(&lisp_as_cons(&root1)->car),
                lisp_as_cons(&lisp_as_cons(&root1)->cdr));
  ASSERT_EQ_PTR(tst,
                lisp_as_cons(&lisp_as_cons(&lisp_as_cons(&root1)->car)->car),
                lisp_as_cons(&root1));
  ASSERT_EQ_PTR(tst,
                lisp_as_cons(&lisp_as_cons(&lisp_as_cons(&root1)->car)->cdr),
                lisp_as_cons(&root1));

  ref_stat.num_leaves = 0;
  ref_stat.num_edges = 4;
  ref_stat.num_white_root_conses = 0;
  ref_stat.num_grey_root_conses = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* set cdr(root) = root 
     root = ( #1 . root )
     #1 = (root, root)
  */
  ASSERT_LISP_OK(tst,
                 lisp_cons_set_cdr(lisp_as_cons(&root1),
                                   &root1));
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_black_root_conses = 1;
  ref_stat.num_grey_root_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_black_conses = 1;
  ref_stat.num_grey_conses = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT_FALSE(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_black_conses = 2;
  ref_stat.num_grey_conses = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_black_conses = 3;
  ref_stat.num_grey_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* swap */
  ASSERT(tst, lisp_vm_gc_swap(&vm));
  ref_stat.num_cycles = 3;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_black_root_conses = 0;
  ref_stat.num_white_conses = 3;
  ref_stat.num_black_conses = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT(tst, lisp_vm_gc_cons_step(&vm));
  ref_stat.num_white_root_conses = 0;
  ref_stat.num_black_root_conses = 1;
  ref_stat.num_black_conses = 0;
  ref_stat.num_white_conses = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* swap */
  ASSERT(tst, lisp_vm_gc_swap(&vm));
  ref_stat.num_cycles = 4;
  ref_stat.num_bulk = 1;
  ref_stat.num_root = 1;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 1;
  ref_stat.num_disposed = 3;
  ref_stat.num_white_root_conses = 1;
  ref_stat.num_black_root_conses = 0;
  ref_stat.num_white_conses = 0;

  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_cons(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "cons");
  TEST(suite, test_make_cons);
  TEST(suite, test_make_cons_and_recyle);
  TEST(suite, test_make_cons_of_conses);
}

