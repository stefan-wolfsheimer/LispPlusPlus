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
#include <unit_tests/common.h>
#include <lisp/util/unit_test.h>
#include <lisp/util/xmalloc.h>
#include <lisp/core/vm.h>
#include <lisp/core/array.h>
#include <lisp/core/error.h>
#include <lisp/core/cell.h>

static void test_make_array(unit_test_t * tst)
{
  lisp_vm_t vm;
  lisp_gc_stat_t ref_stat;
  lisp_cell_t cell[6];
  size_t i;
  memcheck_begin();
  lisp_init_gc_stat(&ref_stat);
  ASSERT_LISP_OK(tst, lisp_init_vm(&vm));
  lisp_vm_gc_set_steps(&vm, 0);
  ASSERT_LISP_OK(tst, lisp_vm_gc_set_cons_page_size(&vm, 4));
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[0] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[0], 1, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[0]), LISP_GC_WHITE_ROOT);

  ref_stat.num_root = 1;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ref_stat.num_reachable = 1;
  ref_stat.num_allocated = 1;
  ref_stat.num_leaves = 1;
  ref_stat.num_void = 0;
  ref_stat.num_cons_pages = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[1] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[1], 4, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[1]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 2;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_leaves = 5;
  ref_stat.num_void = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make arry cell[2] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[2], 3, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[2]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 3;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_leaves = 8;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[3] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[3], 2, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[3]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 4;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 10;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[4] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[4], 0, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[4]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 5;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 5;
  ref_stat.num_reachable = 5;
  ref_stat.num_allocated = 5;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cons objects unqiue */
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), NULL);
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), lisp_as_array(&cell[1]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), lisp_as_array(&cell[2]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), lisp_as_array(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), lisp_as_array(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[1]), lisp_as_array(&cell[2]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[1]), lisp_as_array(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[1]), lisp_as_array(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[2]), lisp_as_array(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[2]), lisp_as_array(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[3]), lisp_as_array(&cell[4]));

  /* unset cell[0] cell[1] cell[2] */
  ASSERT_LISP_OK(tst, lisp_unset(&cell[0]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[1]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[2]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[0]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[1]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[2]));

  ref_stat.num_root = 2;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 2;
  ref_stat.num_objects[LISP_GC_GREY] = 3;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cycle GC */
  ASSERT_LISP_OK(tst, lisp_vm_gc_full_cycle(&vm));
  /*@todo delte array after full cycle!*/
  ref_stat.num_cycles = 1;
  ref_stat.num_root = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_leaves = 2;
  ref_stat.num_disposed = 3;
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[0] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[0], 1, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[0]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 3;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_leaves = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[1] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[1], 5, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[1]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 4;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 8;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make array cell[2] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[2], 3, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[1]), LISP_GC_WHITE_ROOT);
  ref_stat.num_root = 5;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 5;
  ref_stat.num_reachable = 5;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 11;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cell[5] */
  ASSERT_LISP_OK(tst, lisp_make_array(&vm, &cell[5], 2, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell[5]), LISP_GC_WHITE_ROOT);
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), NULL);
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[0]), lisp_as_array(&cell[1]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[1]), lisp_as_array(&cell[2]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[2]), lisp_as_array(&cell[3]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[3]), lisp_as_array(&cell[4]));
  ASSERT_NEQ_PTR(tst, lisp_as_array(&cell[4]), lisp_as_array(&cell[5]));
  ref_stat.num_root = 6;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 6;
  ref_stat.num_reachable = 6;
  ref_stat.num_allocated = 6;
  ref_stat.num_leaves = 13;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset cell[0] cell[1] cell[2] */
  ASSERT_LISP_OK(tst, lisp_unset(&cell[0]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[1]));
  ASSERT_LISP_OK(tst, lisp_unset(&cell[2]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[0]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[1]));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell[2]));

  ref_stat.num_root = 3;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 3;
  ref_stat.num_objects[LISP_GC_GREY] = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_leaves = 4;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cell[3] */
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 2;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cell[4] */
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cell[5] */
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* grey */
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_GREY] = 2;
  ref_stat.num_objects[LISP_GC_BLACK] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_GREY] = 1;
  ref_stat.num_objects[LISP_GC_BLACK] = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ref_stat.num_objects[LISP_GC_BLACK] = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  for(i = 0; i < 6; i++)
  {
    ASSERT_LISP_OK(tst, lisp_unset(&cell[i]));
  }
  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));

  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

#include <lisp/core/complex_object.h>

static void test_make_array_of_arrays(unit_test_t * tst)
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
     child1 = (array nil  nil)
   */
  ASSERT_LISP_OK(tst,
                 lisp_make_array(&vm,
                                 &child1,
                                 2,
                                 &lisp_nil));
  ASSERT(tst, lisp_is_array(&child1));
  ASSERT(tst, lisp_get_cell_color(&child1) == LISP_GC_WHITE_ROOT);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 1u);

  ref_stat.num_root = 1;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ref_stat.num_allocated = 1;
  ref_stat.num_reachable = 1;
  ref_stat.num_leaves = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create second child
     child1 = (array nil  nil)
     child2 = (array nil  nil)
   */
  ASSERT_LISP_OK(tst,
                 lisp_make_array(&vm,
                                 &child2,
                                 2,
                                 &lisp_nil));
  ASSERT(tst, lisp_is_array(&child2));
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_WHITE_ROOT);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child2), 1u);

  ref_stat.num_root = 2;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_leaves = 4;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create array(child1, child2)
     child1 = (array nil  nil)
     child2 = (array nil  nil)
     root = (array (array nil  nil)  (array nil . nil) )
   */
  ASSERT_LISP_OK(tst,
                 lisp_make_array(&vm,
                                 &root1,
                                 0,
                                 NULL));
  ASSERT(tst, lisp_is_array(&root1));
  ASSERT(tst, lisp_is_root_cell(&root1));
  ASSERT_LISP_OK(tst, lisp_array_append(lisp_as_array(&root1), &child1));
  ASSERT_LISP_OK(tst, lisp_array_append(lisp_as_array(&root1), &child2));
  ASSERT_EQ_U(tst, lisp_as_array(&root1)->size, 2u);
  ASSERT(tst, lisp_get_cell_color(&root1) == LISP_GC_WHITE_ROOT);
  ASSERT(tst, lisp_get_cell_color(&child1) == LISP_GC_GREY_ROOT);
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_GREY_ROOT);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&root1), 1u);

  ref_stat.num_root = 3;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ref_stat.num_objects[LISP_GC_GREY_ROOT] = 2;
  ref_stat.num_edges = 2;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset child1, child2
     root = (array (array nil  nil)  (array nil  nil) )
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
  ref_stat.num_objects[LISP_GC_GREY_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_GREY] = 2;
  ref_stat.num_bulk = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_FALSE(tst, lisp_vm_gc_object_step(&vm));
  ASSERT(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 1;
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ref_stat.num_objects[LISP_GC_BLACK] = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* swap */
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_swap(&vm));

  ref_stat.num_cycles = 1;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_WHITE] = 2;
  ref_stat.num_objects[LISP_GC_BLACK] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset root1[0], root1[1]
  */
  ASSERT_LISP_OK(tst, lisp_array_unset(lisp_as_array(&root1), 0));
  ASSERT_LISP_OK(tst, lisp_array_unset(lisp_as_array(&root1), 1));

  ref_stat.num_bulk = 0;
  ref_stat.num_reachable = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_edges = 0;
  ref_stat.num_objects[LISP_GC_WHITE] = 2;
  ref_stat.num_objects[LISP_GC_BLACK] = 0;
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT(tst, lisp_vm_gc_object_step(&vm));

  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 1;
  ref_stat.num_objects[LISP_GC_WHITE] = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* swap */
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_swap(&vm));

  ref_stat.num_cycles = 2;
  ref_stat.num_allocated = 1;
  ref_stat.num_disposed = 2;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 1;
  ref_stat.num_objects[LISP_GC_BLACK_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_WHITE] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);


  /* unset root */
  ASSERT_LISP_OK(tst, lisp_unset(&root1));

  ref_stat.num_root = 0;
  ref_stat.num_reachable = 0;
  ref_stat.num_leaves = 0;
  ref_stat.num_objects[LISP_GC_WHITE_ROOT] = 0;
  ref_stat.num_objects[LISP_GC_GREY] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step */
  ASSERT(tst, lisp_vm_gc_object_step(&vm));
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ref_stat.num_objects[LISP_GC_WHITE] = 0;
  ref_stat.num_objects[LISP_GC_BLACK] = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* swap */
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_swap(&vm));

  ref_stat.num_cycles = 3;
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ref_stat.num_objects[LISP_GC_WHITE] = 1;
  ref_stat.num_objects[LISP_GC_BLACK] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* gc step (nothing done) */
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_object_step(&vm));
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);
  
  /* swap */
  ASSERT(tst, lisp_vm_gc_swappable(&vm));
  ASSERT(tst, lisp_vm_gc_swap(&vm));
  ref_stat.num_cycles = 4;
  ref_stat.num_allocated = 0;
  ref_stat.num_disposed = 3;
  ref_stat.num_objects[LISP_GC_GREY] = 0;
  ref_stat.num_objects[LISP_GC_WHITE] = 0;
  ref_stat.num_objects[LISP_GC_BLACK] = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_LISP_OK(tst, lisp_vm_recycle_next_object(&vm));
  ASSERT_LISP_OK(tst, lisp_vm_recycle_next_object(&vm));
  ASSERT_LISP_OK(tst, lisp_vm_recycle_next_object(&vm));
  ASSERT_EQ_I(tst, lisp_vm_recycle_next_object(&vm), LISP_NO_CHANGE);
  
  
  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_array(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "array");
  TEST(suite, test_make_array);
  TEST(suite, test_make_array_of_arrays);
}
