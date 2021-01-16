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
#include <lisp/core/gc.h>
#include <lisp/core/vm.h>
#include <lisp/core/cons.h>
#include <lisp/core/error.h>

/* @todo move to general include file */
#define LISP_GC_DUMP_TEST LISP_GC_DUMP_HUMAN
/* LISP_GC_DUMP_SILENT 0
   LISP_GC_DUMP_HUMAN 1
*/

#define ASSERT_LISP_OK(__TST__, __EXPR__) \
 ASSERT_EQ_I(__TST__, __EXPR__, LISP_OK);


#define ASSERT_LISP_CHECK_GC(__TST__, __GC__)                           \
  if(!CHECK_EQ_I((__TST__), lisp_gc_check((__GC__)), LISP_OK))          \
  {                                                                     \
    lisp_gc_dump(stdout, (__GC__), LISP_GC_DUMP_TEST);                  \
    return ;                                                            \
  }


static void test_gc_alloc_cons(unit_test_t * tst)
{
  lisp_gc_t gc;
  memcheck_begin();
  ASSERT_LISP_OK(tst, lisp_init_gc(&gc));
  lisp_gc_set_steps(&gc, 0);
  ASSERT_LISP_CHECK_GC(tst, &gc);
  ASSERT_LISP_OK(tst, lisp_gc_set_cons_page_size(&gc, 4));
  ASSERT_EQ_I(tst, gc.num_cons_pages, 0u);

  lisp_cons_t * cons1 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons1), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons1));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons2 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons2), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons2));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons3 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons3), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons3));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons4 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons4), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons4));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons5 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons5), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons5));
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_LISP_CHECK_GC(tst, &gc);

  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);

  lisp_gc_free_cons(&gc, cons1);
  lisp_gc_free_cons(&gc, cons2);
  lisp_gc_free_cons(&gc, cons3);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons1 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons1), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons1));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons2 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons2), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons2));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons3 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons3), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons3));
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons4 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 2u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons4), LISP_GC_WHITE);
  ASSERT_FALSE(tst, lisp_cons_is_root(cons4));
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);
  ASSERT_LISP_CHECK_GC(tst, &gc);
  
  ASSERT_LISP_OK(tst, lisp_free_gc(&gc));

  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_gc_alloc_root_cons(unit_test_t * tst)
{
  lisp_gc_t gc;
  memcheck_begin();
  ASSERT_LISP_OK(tst, lisp_init_gc(&gc));
  lisp_gc_set_steps(&gc, 0);
  ASSERT_LISP_CHECK_GC(tst, &gc);
  ASSERT_LISP_OK(tst, lisp_gc_set_cons_page_size(&gc, 4));
  ASSERT_EQ_I(tst, gc.num_cons_pages, 0u);

  lisp_cons_t * cons1 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons1), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons1));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons2 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons2), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons2));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons3 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons3), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons3));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons4 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons4), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons4));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  lisp_cons_t * cons5 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons5), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons5));
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_LISP_CHECK_GC(tst, &gc);

  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);

  lisp_gc_free_cons(&gc, cons1);
  lisp_gc_free_cons(&gc, cons2);
  lisp_gc_free_cons(&gc, cons3);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons1 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons1), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons1));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons2 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons2), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons2));
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons3 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons3), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons3));
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_LISP_CHECK_GC(tst, &gc);

  cons4 = lisp_gc_alloc_root_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 2u);
  ASSERT_EQ_I(tst, lisp_cons_get_color(cons4), LISP_GC_WHITE);
  ASSERT(tst, lisp_cons_is_root(cons4));
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);
  ASSERT_LISP_CHECK_GC(tst, &gc);
  
  ASSERT_LISP_OK(tst, lisp_free_gc(&gc));

  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_vm_make_cons_of_conses(unit_test_t * tst)
{
  lisp_vm_t vm;
  lisp_cell_t root1;
  lisp_cell_t child1;
  lisp_cell_t child2;

  memcheck_begin();
  ASSERT_LISP_OK(tst, lisp_init_vm(&vm));
  lisp_gc_set_steps(&vm.gc, 0);

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
  ASSERT_LISP_CHECK_GC(tst, &vm.gc);

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
  ASSERT_LISP_CHECK_GC(tst, &vm.gc);

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
  ASSERT_LISP_CHECK_GC(tst, &vm.gc);
  ASSERT(tst, lisp_get_cell_color(&child2) == LISP_GC_GREY);

  /* unset child1, child2 */
  ASSERT_LISP_OK(tst, lisp_unset(&child1));
  ASSERT_LISP_OK(tst, lisp_unset(&child2));
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child1), 0u);
  ASSERT_EQ_U(tst, lisp_get_ref_count(&child2), 0u);
  ASSERT_FALSE(tst, lisp_is_root_cell(&child1));
  ASSERT_FALSE(tst, lisp_is_root_cell(&child2));
  ASSERT_LISP_CHECK_GC(tst, &vm.gc);

  /* @todo gc statistics */

  ASSERT_LISP_OK(tst, lisp_free_vm(&vm));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_gc(unit_context_t * ctx)
{
   unit_suite_t * suite = unit_create_suite(ctx, "gc");
   TEST(suite, test_gc_alloc_cons);
   TEST(suite, test_gc_alloc_root_cons);
   TEST(suite, test_vm_make_cons_of_conses);
}

