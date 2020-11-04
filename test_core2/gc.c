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
#include <lisp/core/cons.h>
#include <lisp/core/error.h>

static void test_gc_alloc_cons(unit_test_t * tst)
{
  lisp_gc_t gc;
  memcheck_begin();
  ASSERT_EQ_I(tst, lisp_init_gc(&gc), LISP_OK);
  ASSERT_EQ_I(tst, lisp_gc_set_cons_page_size(&gc, 4), LISP_OK);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 0u);
  lisp_cons_t * cons1 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  lisp_cons_t * cons2 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  lisp_cons_t * cons3 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  lisp_cons_t * cons4 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 1u);
  lisp_cons_t * cons5 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);

  lisp_gc_free_cons(&gc, cons1);
  lisp_gc_free_cons(&gc, cons2);
  lisp_gc_free_cons(&gc, cons3);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  cons1 = lisp_gc_alloc_cons(&gc);
  cons2 = lisp_gc_alloc_cons(&gc);
  cons3 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 1u);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  cons4 = lisp_gc_alloc_cons(&gc);
  ASSERT_EQ_I(tst, gc.cons_pos, 2u);
  ASSERT_EQ_I(tst, gc.num_cons_pages, 2u);
  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);

  ASSERT_EQ_I(tst, lisp_free_gc(&gc), LISP_OK);

  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_gc(unit_context_t * ctx)
{
   unit_suite_t * suite = unit_create_suite(ctx, "gc");
   TEST(suite, test_gc_alloc_cons);
}

