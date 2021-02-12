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
#include <lisp/util/hash_table.h>
#include <lisp/core/cell.h>
#include <lisp/core/cell_hash_table.h>
#include <lisp/core/cons.h>
#include <lisp/core/error.h>
#include <lisp/core/tid.h>

/* @todo move to general include file */
#define ASSERT_LISP_OK(__TST__, __EXPR__) \
 ASSERT_EQ_I(__TST__, __EXPR__, LISP_OK);

static void test_cell_hash_table(unit_test_t * tst)
{
  int inserted;
  lisp_cell_t input1;
  lisp_cell_t input2;
  lisp_cons_t cons1;
  lisp_cons_t cons2;
  lisp_cell_t * item1;
  lisp_cell_t * item2;
  lisp_cell_t * item3;
  lisp_cell_t * item4;
  lisp_cell_t * item5;
  lisp_cell_t * item6;
  hash_table_t ht;
  memcheck_begin();

  input1.type_id = LISP_TID_CONS;
  input1.data.obj = &cons1;
  input2.type_id = LISP_TID_CONS;
  input2.data.obj = &cons2;

  ASSERT_LISP_OK(tst, lisp_init_cell_hash_table(&ht));

  inserted = 0;
  item1 = lisp_cell_hash_table_find_or_insert(&ht, &lisp_nil, &inserted);
  ASSERT(tst, inserted);

  inserted = 0;
  item2 = lisp_cell_hash_table_find_or_insert(&ht, &lisp_nil, &inserted);
  ASSERT_EQ_PTR(tst, item1, item2);
  ASSERT_FALSE(tst, inserted);

  inserted = 0;
  item3 = lisp_cell_hash_table_find_or_insert(&ht, item1, &inserted);
  ASSERT_EQ_PTR(tst, item1, item3);
  ASSERT_EQ_PTR(tst, item2, item3);
  ASSERT_FALSE(tst, inserted);

  inserted = 0;
  item4 = lisp_cell_hash_table_find_or_insert(&ht, &input1, &inserted);
  ASSERT(tst, lisp_is_cons(item4));
  ASSERT_EQ_PTR(tst, item4->data.obj, &cons1);
  ASSERT(tst, inserted);

  inserted = 0;
  item5 = lisp_cell_hash_table_find_or_insert(&ht, &input2, &inserted);
  ASSERT(tst, lisp_is_cons(item5));
  ASSERT_EQ_PTR(tst, item5->data.obj, &cons2);
  ASSERT(tst, inserted);

  inserted = 0;
  item6 = lisp_cell_hash_table_find_or_insert(&ht, &input2, &inserted);
  ASSERT(tst, lisp_is_cons(item6));
  ASSERT_EQ_PTR(tst, item6->data.obj, &cons2);
  ASSERT_FALSE(tst, inserted);
  ASSERT_EQ_PTR(tst, item6, item5);

  lisp_free_cell_hash_table(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_cell(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "cell");
  TEST(suite, test_cell_hash_table);
}
