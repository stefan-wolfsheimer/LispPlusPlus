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
#include <lisp/core/gc_stat.h>
#include <lisp/core/cons.h>
#include <lisp/core/error.h>

/* @todo move to general include file */
#define LISP_GC_DUMP_TEST LISP_GC_DUMP_HUMAN
/* LISP_GC_DUMP_SILENT 0
   LISP_GC_DUMP_HUMAN 1
*/

#define ASSERT_LISP_OK(__TST__, __EXPR__) \
 ASSERT_EQ_I(__TST__, __EXPR__, LISP_OK);


#define ASSERT_LISP_CHECK_GC(__TST__, __VM__)                           \
  if(!CHECK_EQ_I((__TST__), lisp_vm_gc_check((__VM__)), LISP_OK))       \
  {                                                                     \
    lisp_vm_gc_dump(stdout, (__VM__), LISP_GC_DUMP_TEST);               \
    return ;                                                            \
  }

#define ASSERT_LISP_CHECK_GC_STATS(__TST__, __VM__, __REQ__)            \
{                                                                       \
   lisp_gc_stat_t _stat_;                                               \
   int _stat_ok_ = 1;                                                   \
   lisp_vm_gc_get_stats((__VM__), &_stat_);                             \
   if(!CHECK((__TST__), lisp_gc_stat_eq(&_stat_, (__REQ__))))           \
   {                                                                    \
     lisp_gc_stat_print2(stdout, &_stat_, (__REQ__));                   \
     _stat_ok_ = 0;                                                     \
   }                                                                    \
   if(!CHECK_EQ_I((__TST__), lisp_vm_gc_check((__VM__)), LISP_OK))      \
   {                                                                    \
     lisp_vm_gc_dump(stdout, (__VM__), LISP_GC_DUMP_TEST);              \
     _stat_ok_ = 0;                                                     \
   }                                                                    \
   if(!_stat_ok_)                                                       \
   {                                                                    \
     return;                                                            \
   }                                                                    \
 }

static void test_vm_alloc_root_cons(unit_test_t * tst)
{
  lisp_vm_t vm;
  lisp_gc_stat_t ref_stat;
  lisp_cell_t cell1;
  lisp_cell_t cell2;
  lisp_cell_t cell3;
  lisp_cell_t cell4;
  lisp_cell_t cell5;
  lisp_cell_t cell6;

  memcheck_begin();
  lisp_init_gc_stat(&ref_stat);
  ASSERT_LISP_OK(tst, lisp_init_vm(&vm));
  lisp_vm_gc_set_steps(&vm, 0);
  ASSERT_LISP_OK(tst, lisp_vm_gc_set_cons_page_size(&vm, 4));
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell1 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell1, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell1), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell1));
  ref_stat.num_root = 1;
  ref_stat.num_reachable = 1;
  ref_stat.num_allocated = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_void = 3;
  ref_stat.num_cons_pages = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell2 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell2, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell2), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell2));
  ref_stat.num_root = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_allocated = 2;
  ref_stat.num_leaves = 4;
  ref_stat.num_void = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell3 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell3, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell3), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell3));
  ref_stat.num_root = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_leaves = 6;
  ref_stat.num_void = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell4 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell4, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell4), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell4));
  ref_stat.num_root = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 8;
  ref_stat.num_void = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell5 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell5, NULL, NULL));
  ASSERT(tst, lisp_is_root_cell(&cell5));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell5), LISP_GC_WHITE);
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 5;
  ref_stat.num_reachable = 5;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 10;
  ref_stat.num_void = 3;
  ref_stat.num_cons_pages = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cons objects unqiue */
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), NULL);
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), lisp_as_cons(&cell2));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), lisp_as_cons(&cell3));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), lisp_as_cons(&cell4));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), lisp_as_cons(&cell5));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell2), lisp_as_cons(&cell3));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell2), lisp_as_cons(&cell4));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell2), lisp_as_cons(&cell5));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell3), lisp_as_cons(&cell4));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell3), lisp_as_cons(&cell5));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell4), lisp_as_cons(&cell5));

  /* unset cell1 cell2 cell3 */
  ASSERT_LISP_OK(tst, lisp_unset(&cell1));
  ASSERT_LISP_OK(tst, lisp_unset(&cell2));
  ASSERT_LISP_OK(tst, lisp_unset(&cell3));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell1));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell2));
  ASSERT_FALSE(tst, lisp_is_root_cell(&cell3));

  ref_stat.num_root = 2;
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
  ref_stat.num_recycled = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);

  /* make cons cell1 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell1, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell1), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell1));
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_allocated = 3;
  ref_stat.num_leaves = 6;
  ref_stat.num_recycled = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell2 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell2, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell2), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell2));
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 4;
  ref_stat.num_reachable = 4;
  ref_stat.num_allocated = 4;
  ref_stat.num_leaves = 8;
  ref_stat.num_recycled = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* make cons cell3 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell3, NULL, NULL));
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell2), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell3));
  ASSERT_EQ_I(tst, vm.cons_pos, 1u);
  ref_stat.num_root = 5;
  ref_stat.num_reachable = 5;
  ref_stat.num_allocated = 5;
  ref_stat.num_leaves = 10;
  ref_stat.num_recycled = 0;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* cell6 */
  ASSERT_LISP_OK(tst, lisp_make_cons(&vm, &cell6, NULL, NULL));
  ASSERT_EQ_I(tst, vm.cons_pos, 2u);
  ASSERT_EQ_I(tst, lisp_get_cell_color(&cell6), LISP_GC_WHITE);
  ASSERT(tst, lisp_is_root_cell(&cell6));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), NULL);
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell1), lisp_as_cons(&cell2));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell2), lisp_as_cons(&cell3));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell3), lisp_as_cons(&cell4));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell4), lisp_as_cons(&cell5));
  ASSERT_NEQ_PTR(tst, lisp_as_cons(&cell5), lisp_as_cons(&cell6));
  ref_stat.num_root = 6;
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

static void test_vm_make_cons_of_conses(unit_test_t * tst)
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

  /* create first child */
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
  ref_stat.num_allocated = 1;
  ref_stat.num_reachable = 1;
  ref_stat.num_leaves = 2;
  ref_stat.num_cons_pages = 1;
  ref_stat.num_void = 3;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create second child */
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
  ref_stat.num_allocated = 2;
  ref_stat.num_reachable = 2;
  ref_stat.num_leaves = 4;
  ref_stat.num_cons_pages = 1;
  ref_stat.num_void = 2;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* create cons(child1, child2)  */
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
  ref_stat.num_allocated = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_leaves = 4;
  ref_stat.num_edges = 2;
  ref_stat.num_cons_pages = 1;
  ref_stat.num_void = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  /* unset child1, child2 */
  ASSERT_LISP_OK(tst, lisp_unset(&child1));
  ASSERT_LISP_OK(tst, lisp_unset(&child2));
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 0u);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child2), 0u);
  ASSERT_FALSE(tst, lisp_is_root_cell(&child1));
  ASSERT_FALSE(tst, lisp_is_root_cell(&child2));

  ref_stat.num_root = 1;
  ref_stat.num_bulk = 2;
  ref_stat.num_allocated = 3;
  ref_stat.num_reachable = 3;
  ref_stat.num_leaves = 4;
  ref_stat.num_edges = 2;
  ref_stat.num_cons_pages = 1;
  ASSERT_LISP_CHECK_GC_STATS(tst, &vm, &ref_stat);

  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_vm(unit_context_t * ctx)
{
   unit_suite_t * suite = unit_create_suite(ctx, "vm");
   TEST(suite, test_vm_alloc_root_cons);
   TEST(suite, test_vm_make_cons_of_conses);
}

