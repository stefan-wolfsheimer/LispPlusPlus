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
#include <lisp/core/cons.h>

static void test_cons_alloc(unit_test_t * tst)
{
  lisp_cons_pages_t pages;
  ASSERT_FALSE(tst, lisp_init_cons_pages(&pages));
  ASSERT_FALSE(tst, lisp_cons_pages_set_size(&pages, 4));
  ASSERT_EQ_I(tst, pages.num_pages, 0u);
  lisp_cons_t * cons1 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.num_pages, 1u);
  lisp_cons_t * cons2 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.num_pages, 1u);
  lisp_cons_t * cons3 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.num_pages, 1u);
  lisp_cons_t * cons4 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.num_pages, 1u);
  lisp_cons_t * cons5 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.num_pages, 2u);
  ASSERT_EQ_I(tst, pages.pos, 1u);
  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);

  lisp_cons_free(&pages, cons1);
  lisp_cons_free(&pages, cons2);
  lisp_cons_free(&pages, cons3);
  ASSERT_EQ_I(tst, pages.pos, 1u);
  cons1 = lisp_cons_alloc(&pages);
  cons2 = lisp_cons_alloc(&pages);
  cons3 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.pos, 1u);
  cons4 = lisp_cons_alloc(&pages);
  ASSERT_EQ_I(tst, pages.pos, 2u);
  ASSERT_NEQ_PTR(tst, cons1, NULL);
  ASSERT_NEQ_PTR(tst, cons1, cons2);
  ASSERT_NEQ_PTR(tst, cons2, cons3);
  ASSERT_NEQ_PTR(tst, cons3, cons4);
  ASSERT_NEQ_PTR(tst, cons4, cons5);
}

void test_cons(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "cons");
  TEST(suite, test_cons_alloc);
}

