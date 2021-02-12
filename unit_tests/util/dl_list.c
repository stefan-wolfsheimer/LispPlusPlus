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
#include <lisp/util/dl_list.h>
#include <lisp/util/unit_test.h>

void test_dl_append_first(unit_test_t * tst)
{
  lisp_dl_list_t ll;
  lisp_dl_item_t item1;
  lisp_init_dl_list(&ll);
  ASSERT(tst, lisp_dl_list_empty(&ll));

  /* append first */
  lisp_dl_list_append(&ll, &item1);
  ASSERT_FALSE(tst, lisp_dl_list_empty(&ll));
  ASSERT_EQ_PTR(tst, ll.first, &item1);
  ASSERT_EQ_PTR(tst, ll.last, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, NULL);
  ASSERT_EQ_PTR(tst, item1.next, NULL);
}

void test_dl_prepend_first(unit_test_t * tst)
{
  lisp_dl_list_t ll;
  lisp_dl_item_t item1;
  lisp_init_dl_list(&ll);
  
  /* prepend first */
  lisp_dl_list_prepend(&ll, &item1);
  ASSERT_EQ_PTR(tst, ll.first, &item1);
  ASSERT_EQ_PTR(tst, ll.last, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, NULL);
  ASSERT_EQ_PTR(tst, item1.next, NULL);

}

void test_dl_list_lifetime(unit_test_t * tst)
{
  lisp_dl_list_t ll;
  lisp_dl_item_t item_1;
  lisp_dl_item_t item0;
  lisp_dl_item_t item1;
  lisp_dl_item_t item2;
  lisp_dl_item_t item3;
  lisp_dl_item_t item4;
  lisp_init_dl_list(&ll);
  lisp_dl_list_append(&ll, &item1);
  lisp_dl_list_append(&ll, &item2);
  ASSERT_EQ_PTR(tst, ll.first, &item1);
  ASSERT_EQ_PTR(tst, ll.last, &item2);
  ASSERT_EQ_PTR(tst, item1.prev, NULL);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, NULL);

  lisp_dl_list_append(&ll, &item3);
  ASSERT_EQ_PTR(tst, ll.first, &item1);
  ASSERT_EQ_PTR(tst, ll.last, &item3);
  ASSERT_EQ_PTR(tst, item1.prev, NULL);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item2);
  ASSERT_EQ_PTR(tst, item3.next, NULL);

  lisp_dl_list_prepend(&ll, &item0);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item3);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, &item0);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item2);
  ASSERT_EQ_PTR(tst, item3.next, NULL);

  /** insert after last */
  lisp_dl_list_insert(&ll, &item3, &item4);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item4);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, &item0);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item2);
  ASSERT_EQ_PTR(tst, item3.next, &item4);
  ASSERT_EQ_PTR(tst, item4.prev, &item3);
  ASSERT_EQ_PTR(tst, item4.next, NULL);

  /** insert before first */
  lisp_dl_list_insert(&ll, NULL, &item_1);
  ASSERT_EQ_PTR(tst, ll.first, &item_1);
  ASSERT_EQ_PTR(tst, ll.last, &item4);
  ASSERT_EQ_PTR(tst, item_1.prev, NULL);
  ASSERT_EQ_PTR(tst, item_1.next, &item0);
  ASSERT_EQ_PTR(tst, item0.prev, &item_1);
  ASSERT_EQ_PTR(tst, item0.next, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, &item0);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item2);
  ASSERT_EQ_PTR(tst, item3.next, &item4);
  ASSERT_EQ_PTR(tst, item4.prev, &item3);
  ASSERT_EQ_PTR(tst, item4.next, NULL);

  /** remove first */
  lisp_dl_list_remove(&ll, &item_1);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item4);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, &item0);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item2);
  ASSERT_EQ_PTR(tst, item3.next, &item4);
  ASSERT_EQ_PTR(tst, item4.prev, &item3);
  ASSERT_EQ_PTR(tst, item4.next, NULL);

  /** remove last */
  lisp_dl_list_remove(&ll, &item4);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item3);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, &item0);
  ASSERT_EQ_PTR(tst, item1.next, &item2);
  ASSERT_EQ_PTR(tst, item2.prev, &item1);
  ASSERT_EQ_PTR(tst, item2.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item2);
  ASSERT_EQ_PTR(tst, item3.next, NULL);

  lisp_dl_list_remove(&ll, &item2);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item3);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, &item1);
  ASSERT_EQ_PTR(tst, item1.prev, &item0);
  ASSERT_EQ_PTR(tst, item1.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item1);
  ASSERT_EQ_PTR(tst, item3.next, NULL);

  lisp_dl_list_remove(&ll, &item1);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item3);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, &item3);
  ASSERT_EQ_PTR(tst, item3.prev, &item0);
  ASSERT_EQ_PTR(tst, item3.next, NULL);

  lisp_dl_list_remove(&ll, &item3);
  ASSERT_EQ_PTR(tst, ll.first, &item0);
  ASSERT_EQ_PTR(tst, ll.last, &item0);
  ASSERT_EQ_PTR(tst, item0.prev, NULL);
  ASSERT_EQ_PTR(tst, item0.next, NULL);


  lisp_dl_list_remove(&ll, &item0);
  ASSERT_EQ_PTR(tst, ll.first, NULL);
  ASSERT_EQ_PTR(tst, ll.last, NULL);
  ASSERT(tst, lisp_dl_list_empty(&ll));
}

void test_dl_list(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "dl_list");
  TEST(suite, test_dl_append_first);
  TEST(suite, test_dl_prepend_first);
  TEST(suite, test_dl_list_lifetime);
}
