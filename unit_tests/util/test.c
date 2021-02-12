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
#include <stdio.h>

/*********************************************************************
 * 
 * check functions for different types
 *
 *********************************************************************/
static void test_check_boolean(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  unit_test_t * tst1   = unit_create_test(suite, "test1", test_check_boolean);

  ASSERT(tst, 1);
  ASSERT(tst,       CHECK(tst1, 1));
  ASSERT(tst,       CHECK(NULL, 1));
  ASSERT_FALSE(tst, CHECK(tst1, 0));
  ASSERT_FALSE(tst, CHECK(NULL, 0));

  ASSERT_FALSE(tst, CHECK_FALSE(tst1, 1));
  ASSERT_FALSE(tst, CHECK_FALSE(NULL, 1));
  ASSERT(tst,       CHECK_FALSE(tst1, 0));
  ASSERT(tst,       CHECK_FALSE(NULL, 0));

  ASSERT(tst,       CHECK_EQ_U(tst1, 1,1));
  ASSERT(tst,       CHECK_EQ_U(NULL, 1,1));
  ASSERT_FALSE(tst, CHECK_EQ_U(tst1, 0,1));
  ASSERT_FALSE(tst, CHECK_EQ_U(NULL, 0,1));

  unit_free_context(ctx);
}

static void test_check_int(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t   * suite  = unit_create_suite(ctx, "suite1");
  unit_test_t    * tst1   = unit_create_test(suite, "test1", test_check_int);

  /* EQ == */
  ASSERT_EQ_I(tst, -1, -1);
  ASSERT(tst,       CHECK_EQ_I(tst1, -1, -1));
  ASSERT_FALSE(tst, CHECK_EQ_I(tst1, -1, 2));
  ASSERT_FALSE(tst, CHECK_EQ_I(tst1, 2, -1));
  ASSERT(tst,       CHECK_EQ_I(NULL, -1, -1));
  ASSERT_FALSE(tst, CHECK_EQ_I(NULL, -1, 2));
  ASSERT_FALSE(tst, CHECK_EQ_I(NULL, 2, -1));

  /* NEQ != */
  ASSERT_NEQ_I(tst, -1, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_I(tst1, -1, -1));
  ASSERT(tst,       CHECK_NEQ_I(tst1, -1, 2));
  ASSERT(tst,       CHECK_NEQ_I(tst1, 2, -1));
  ASSERT_FALSE(tst, CHECK_NEQ_I(NULL, -1, -1));
  ASSERT(tst,       CHECK_NEQ_I(NULL, -1, 2));
  ASSERT(tst,       CHECK_NEQ_I(NULL, 2, -1));

  /* GT > */
  ASSERT_GT_I(tst, 2, -1);
  ASSERT_FALSE(tst, CHECK_GT_I(tst1, -1, -1));
  ASSERT_FALSE(tst, CHECK_GT_I(tst1, -1, 2));
  ASSERT(tst,       CHECK_GT_I(tst1, 2, -1));
  ASSERT_FALSE(tst, CHECK_GT_I(NULL, -1, -1));
  ASSERT_FALSE(tst, CHECK_GT_I(NULL, -1, 2));
  ASSERT(tst,       CHECK_GT_I(NULL, 2, -1));

  /* GE >= */
  ASSERT_GE_I(tst, 2, -1);
  ASSERT_GE_I(tst, 2, 2);
  ASSERT(tst,       CHECK_GE_I(tst1, -1, -1));
  ASSERT_FALSE(tst, CHECK_GE_I(tst1, -1, 2));
  ASSERT(tst,       CHECK_GE_I(tst1, 2, -1));
  ASSERT(tst,       CHECK_GE_I(NULL, -1, -1));
  ASSERT_FALSE(tst, CHECK_GE_I(NULL, -1, 2));
  ASSERT(tst,       CHECK_GE_I(NULL, 2, -1));

     
  /* LT < */
  ASSERT_LT_I(tst, -1, 2);
  ASSERT_FALSE(tst, CHECK_LT_I(tst1, -1, -1));
  ASSERT(tst,       CHECK_LT_I(tst1, -1, 2));
  ASSERT_FALSE(tst, CHECK_LT_I(tst1, 2, -1));
  ASSERT_FALSE(tst, CHECK_LT_I(NULL, -1, -1));
  ASSERT(tst,       CHECK_LT_I(NULL, -1, 2));
  ASSERT_FALSE(tst, CHECK_LT_I(NULL, 2, -1));

  /* LE <= */
  ASSERT_LE_I(tst, -1, 2);
  ASSERT_LE_I(tst, -1, -1);
  ASSERT(tst,       CHECK_LE_I(tst1, -1, -1));
  ASSERT(tst,       CHECK_LE_I(tst1, -1, 2));
  ASSERT_FALSE(tst, CHECK_LE_I(tst1, 2, -1));
  ASSERT(tst,       CHECK_LE_I(NULL, -1, -1));
  ASSERT(tst,       CHECK_LE_I(NULL, -1, 2));
  ASSERT_FALSE(tst, CHECK_LE_I(NULL, 2, -1));

  unit_free_context(ctx);
}

static void test_check_arr_int(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t   * suite  = unit_create_suite(ctx, "suite1");
  unit_test_t    * tst1   = unit_create_test(suite, "test1", test_check_arr_int);
  const int a1[3] = { 1, 2, 3};
  const int a2[2] = { 4, 5 };

  /* EQ == */
  ASSERT_EQ_ARR_I(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_EQ_ARR_I(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_I(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_I(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_EQ_ARR_I(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_I(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_I(NULL, a2, 2, a1, 3));

  /* NEQ != */
  ASSERT_NEQ_ARR_I(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_I(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_I(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_I(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_I(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_I(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_I(NULL, a2, 2, a1, 3));

  /* GT > */
  ASSERT_GT_ARR_I(tst, a2, 2, a1, 3);
  ASSERT_FALSE(tst, CHECK_GT_ARR_I(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_I(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_I(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_I(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_I(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_I(NULL, a2, 2, a1, 3));

  /* GE >= */
  ASSERT_GE_ARR_I(tst, a2, 2, a1, 3);
  ASSERT_GE_ARR_I(tst, a2, 2, a2, 2);
  ASSERT(tst,       CHECK_GE_ARR_I(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_I(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_I(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_GE_ARR_I(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_I(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_I(NULL, a2, 2, a1, 3));

     
  /* LT < */
  ASSERT_LT_ARR_I(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_LT_ARR_I(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_I(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_I(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_LT_ARR_I(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_I(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_I(NULL, a2, 2, a1, 3));

  /* LE <= */
  ASSERT_LE_ARR_I(tst, a1, 3, a2, 2);
  ASSERT_LE_ARR_I(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_LE_ARR_I(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_I(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_I(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_I(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_I(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_I(NULL, a2, 2, a1, 3));

  unit_free_context(ctx);
}

static void test_check_unsigned_int(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t   * suite  = unit_create_suite(ctx, "suite1");
  unit_test_t    * tst1   = unit_create_test(suite, "test1", test_check_unsigned_int);

  /* EQ == */
  ASSERT_EQ_U(tst, 1, 1);
  ASSERT(tst,       CHECK_EQ_U(tst1, 1, 1));
  ASSERT_FALSE(tst, CHECK_EQ_U(tst1, 1, 2));
  ASSERT_FALSE(tst, CHECK_EQ_U(tst1, 2, 1));
  ASSERT(tst,       CHECK_EQ_U(NULL, 1, 1));
  ASSERT_FALSE(tst, CHECK_EQ_U(NULL, 1, 2));
  ASSERT_FALSE(tst, CHECK_EQ_U(NULL, 2, 1));

  /* NEQ != */
  ASSERT_NEQ_U(tst, 1, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_U(tst1, 1, 1));
  ASSERT(tst,       CHECK_NEQ_U(tst1, 1, 2));
  ASSERT(tst,       CHECK_NEQ_U(tst1, 2, 1));
  ASSERT_FALSE(tst, CHECK_NEQ_U(NULL, 1, 1));
  ASSERT(tst,       CHECK_NEQ_U(NULL, 1, 2));
  ASSERT(tst,       CHECK_NEQ_U(NULL, 2, 1));

  /* GT > */
  ASSERT_GT_U(tst, 2, 1);
  ASSERT_FALSE(tst, CHECK_GT_U(tst1, 1, 1));
  ASSERT_FALSE(tst, CHECK_GT_U(tst1, 1, 2));
  ASSERT(tst,       CHECK_GT_U(tst1, 2, 1));
  ASSERT_FALSE(tst, CHECK_GT_U(NULL, 1, 1));
  ASSERT_FALSE(tst, CHECK_GT_U(NULL, 1, 2));
  ASSERT(tst,       CHECK_GT_U(NULL, 2, 1));

  /* GE >= */
  ASSERT_GE_U(tst, 2, 1);
  ASSERT_GE_U(tst, 2, 2);
  ASSERT(tst,       CHECK_GE_U(tst1, 1, 1));
  ASSERT_FALSE(tst, CHECK_GE_U(tst1, 1, 2));
  ASSERT(tst,       CHECK_GE_U(tst1, 2, 1));
  ASSERT(tst,       CHECK_GE_U(NULL, 1, 1));
  ASSERT_FALSE(tst, CHECK_GE_U(NULL, 1, 2));
  ASSERT(tst,       CHECK_GE_U(NULL, 2, 1));

     
  /* LT < */
  ASSERT_LT_U(tst, 1, 2);
  ASSERT_FALSE(tst, CHECK_LT_U(tst1, 1, 1));
  ASSERT(tst,       CHECK_LT_U(tst1, 1, 2));
  ASSERT_FALSE(tst, CHECK_LT_U(tst1, 2, 1));
  ASSERT_FALSE(tst, CHECK_LT_U(NULL, 1, 1));
  ASSERT(tst,       CHECK_LT_U(NULL, 1, 2));
  ASSERT_FALSE(tst, CHECK_LT_U(NULL, 2, 1));

  /* LE <= */
  ASSERT_LE_U(tst, 1, 2);
  ASSERT_LE_U(tst, 1, 1);
  ASSERT(tst,       CHECK_LE_U(tst1, 1, 1));
  ASSERT(tst,       CHECK_LE_U(tst1, 1, 2));
  ASSERT_FALSE(tst, CHECK_LE_U(tst1, 2, 1));
  ASSERT(tst,       CHECK_LE_U(NULL, 1, 1));
  ASSERT(tst,       CHECK_LE_U(NULL, 1, 2));
  ASSERT_FALSE(tst, CHECK_LE_U(NULL, 2, 1));

  unit_free_context(ctx);
}

static void test_check_arr_unsigned_int(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t   * suite  = unit_create_suite(ctx, "suite1");
  unit_test_t    * tst1   = unit_create_test(suite, "test1", 
					     test_check_arr_unsigned_int);
  const unsigned int a1[3] = { 1, 2, 3};
  const unsigned int a2[2] = { 4, 5 };

  /* EQ == */
  ASSERT_EQ_ARR_U(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_EQ_ARR_U(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_U(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_U(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_EQ_ARR_U(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_U(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_U(NULL, a2, 2, a1, 3));

  /* NEQ != */
  ASSERT_NEQ_ARR_U(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_U(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_U(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_U(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_U(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_U(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_U(NULL, a2, 2, a1, 3));

  /* GT > */
  ASSERT_GT_ARR_U(tst, a2, 2, a1, 3);
  ASSERT_FALSE(tst, CHECK_GT_ARR_U(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_U(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_U(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_U(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_U(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_U(NULL, a2, 2, a1, 3));

  /* GE >= */
  ASSERT_GE_ARR_U(tst, a2, 2, a1, 3);
  ASSERT_GE_ARR_U(tst, a2, 2, a2, 2);
  ASSERT(tst,       CHECK_GE_ARR_U(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_U(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_U(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_GE_ARR_U(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_U(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_U(NULL, a2, 2, a1, 3));

     
  /* LT < */
  ASSERT_LT_ARR_U(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_LT_ARR_U(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_U(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_U(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_LT_ARR_U(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_U(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_U(NULL, a2, 2, a1, 3));

  /* LE <= */
  ASSERT_LE_ARR_U(tst, a1, 3, a2, 2);
  ASSERT_LE_ARR_U(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_LE_ARR_U(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_U(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_U(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_U(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_U(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_U(NULL, a2, 2, a1, 3));

  unit_free_context(ctx);
}


static void test_check_ptr(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  unit_test_t * tst1   = unit_create_test(suite, "test1", test_check_unsigned_int);
  int * v = malloc(sizeof(int)*3);

  /* EQ == */
  ASSERT_EQ_PTR(tst, v+1, v+1);
  ASSERT(tst,       CHECK_EQ_PTR(tst1, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_EQ_PTR(tst1, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_EQ_PTR(tst1, v+2, v+1));
  ASSERT(tst,       CHECK_EQ_PTR(NULL, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_EQ_PTR(NULL, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_EQ_PTR(NULL, v+2, v+1));

  /* NEQ != */
  ASSERT_NEQ_PTR(tst, v+1, v+2);
  ASSERT_FALSE(tst, CHECK_NEQ_PTR(tst1, v+1, v+1));
  ASSERT(tst,       CHECK_NEQ_PTR(tst1, v+1, v+2));
  ASSERT(tst,       CHECK_NEQ_PTR(tst1, v+2, v+1));
  ASSERT_FALSE(tst, CHECK_NEQ_PTR(NULL, v+1, v+1));
  ASSERT(tst,       CHECK_NEQ_PTR(NULL, v+1, v+2));
  ASSERT(tst,       CHECK_NEQ_PTR(NULL, v+2, v+1));


  /* GT > */
  ASSERT_GT_PTR(tst, v+2, v+1);
  ASSERT_FALSE(tst, CHECK_GT_PTR(tst1, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_GT_PTR(tst1, v+1, v+2));
  ASSERT(tst,       CHECK_GT_PTR(tst1, v+2, v+1));
  ASSERT_FALSE(tst, CHECK_GT_PTR(NULL, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_GT_PTR(NULL, v+1, v+2));
  ASSERT(tst,       CHECK_GT_PTR(NULL, v+2, v+1));

  /* GE >= */
  ASSERT_GE_PTR(tst, v+2, v+1);
  ASSERT_GE_PTR(tst, v+2, v+2);
  ASSERT(tst,       CHECK_GE_PTR(tst1, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_GE_PTR(tst1, v+1, v+2));
  ASSERT(tst,       CHECK_GE_PTR(tst1, v+2, v+1));
  ASSERT(tst,       CHECK_GE_PTR(NULL, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_GE_PTR(NULL, v+1, v+2));
  ASSERT(tst,       CHECK_GE_PTR(NULL, v+2, v+1));

     
  /* LT < */
  ASSERT_LT_PTR(tst, v+1, v+2);
  ASSERT_FALSE(tst, CHECK_LT_PTR(tst1, v+1, v+1));
  ASSERT(tst,       CHECK_LT_PTR(tst1, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_LT_PTR(tst1, v+2, v+1));
  ASSERT_FALSE(tst, CHECK_LT_PTR(NULL, v+1, v+1));
  ASSERT(tst,       CHECK_LT_PTR(NULL, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_LT_PTR(NULL, v+2, v+1));


  /* LE <= */
  ASSERT_LE_PTR(tst, v+1, v+2);
  ASSERT_LE_PTR(tst, v+1, v+1);
  ASSERT(tst,       CHECK_LE_PTR(tst1, v+1, v+1));
  ASSERT(tst,       CHECK_LE_PTR(tst1, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_LE_PTR(tst1, v+2, v+1));
  ASSERT(tst,       CHECK_LE_PTR(NULL, v+1, v+1));
  ASSERT(tst,       CHECK_LE_PTR(NULL, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_LE_PTR(NULL, v+2, v+1));

  free(v);
  unit_free_context(ctx);
}

static void test_check_arr_ptr(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t   * suite  = unit_create_suite(ctx, "suite1");
  unit_test_t    * tst1   = unit_create_test(suite, "test1", 
					     test_check_arr_unsigned_int);
  int * v = malloc(sizeof(int)*5);
  const int * a1[3] = { NULL, NULL, NULL };
  const int * a2[2] = { NULL, NULL };


  a1[0] = v + 0;
  a1[1] = v + 1;
  a1[2] = v + 2;

  a2[0] = v + 3;
  a2[1] = v + 4;

  /* EQ == */
  ASSERT_EQ_ARR_PTR(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_EQ_ARR_PTR(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_PTR(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_PTR(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_EQ_ARR_PTR(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_PTR(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_PTR(NULL, a2, 2, a1, 3));

  /* NEQ != */
  ASSERT_NEQ_ARR_PTR(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_PTR(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_PTR(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_PTR(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_PTR(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_PTR(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_PTR(NULL, a2, 2, a1, 3));

  /* GT > */
  ASSERT_GT_ARR_PTR(tst, a2, 2, a1, 3);
  ASSERT_FALSE(tst, CHECK_GT_ARR_PTR(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_PTR(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_PTR(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_PTR(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_PTR(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_PTR(NULL, a2, 2, a1, 3));

  /* GE >= */
  ASSERT_GE_ARR_PTR(tst, a2, 2, a1, 3);
  ASSERT_GE_ARR_PTR(tst, a2, 2, a2, 2);
  ASSERT(tst,       CHECK_GE_ARR_PTR(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_PTR(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_PTR(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_GE_ARR_PTR(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_PTR(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_PTR(NULL, a2, 2, a1, 3));

     
  /* LT < */
  ASSERT_LT_ARR_PTR(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_LT_ARR_PTR(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_PTR(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_PTR(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_LT_ARR_PTR(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_PTR(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_PTR(NULL, a2, 2, a1, 3));

  /* LE <= */
  ASSERT_LE_ARR_PTR(tst, a1, 3, a2, 2);
  ASSERT_LE_ARR_PTR(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_LE_ARR_PTR(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_PTR(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_PTR(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_PTR(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_PTR(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_PTR(NULL, a2, 2, a1, 3));

  free(v);
  unit_free_context(ctx);
}

static void test_check_cstr(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  unit_test_t * tst1   = unit_create_test(suite, "test1", test_check_cstr);

  /* EQ == */
  ASSERT_EQ_CSTR(tst, "abc", "abc");
  ASSERT(tst,       CHECK_EQ_CSTR(tst1, "abc", "abc"));
  ASSERT_FALSE(tst, CHECK_EQ_CSTR(tst1, "abc", "def"));
  ASSERT_FALSE(tst, CHECK_EQ_CSTR(tst1, "def", "abc"));
  ASSERT(tst,       CHECK_EQ_CSTR(NULL, "abc", "abc"));
  ASSERT_FALSE(tst, CHECK_EQ_CSTR(NULL, "abc", "def"));
  ASSERT_FALSE(tst, CHECK_EQ_CSTR(NULL, "def", "abc"));

  /* NEQ != */
  ASSERT_NEQ_CSTR(tst, "abc", "def");
  ASSERT_FALSE(tst, CHECK_NEQ_CSTR(tst1, "abc", "abc"));
  ASSERT(tst,       CHECK_NEQ_CSTR(tst1, "abc", "def"));
  ASSERT(tst,       CHECK_NEQ_CSTR(tst1, "def", "abc"));
  ASSERT_FALSE(tst, CHECK_NEQ_CSTR(NULL, "abc", "abc"));
  ASSERT(tst,       CHECK_NEQ_CSTR(NULL, "abc", "def"));
  ASSERT(tst,       CHECK_NEQ_CSTR(NULL, "def", "abc"));


  /* GT > */
  ASSERT_GT_CSTR(tst, "def", "abc");
  ASSERT_FALSE(tst, CHECK_GT_CSTR(tst1, "abc", "abc"));
  ASSERT_FALSE(tst, CHECK_GT_CSTR(tst1, "abc", "def"));
  ASSERT(tst,       CHECK_GT_CSTR(tst1, "def", "abc"));
  ASSERT_FALSE(tst, CHECK_GT_CSTR(NULL, "abc", "abc"));
  ASSERT_FALSE(tst, CHECK_GT_CSTR(NULL, "abc", "def"));
  ASSERT(tst,       CHECK_GT_CSTR(NULL, "def", "abc"));

  /* GE >= */
  ASSERT_GE_CSTR(tst, "def", "abc");
  ASSERT_GE_CSTR(tst, "def", "def");
  ASSERT(tst,       CHECK_GE_CSTR(tst1, "abc", "abc"));
  ASSERT_FALSE(tst, CHECK_GE_CSTR(tst1, "abc", "def"));
  ASSERT(tst,       CHECK_GE_CSTR(tst1, "def", "abc"));
  ASSERT(tst,       CHECK_GE_CSTR(NULL, "abc", "abc"));
  ASSERT_FALSE(tst, CHECK_GE_CSTR(NULL, "abc", "def"));
  ASSERT(tst,       CHECK_GE_CSTR(NULL, "def", "abc"));

     
  /* LT < */
  ASSERT_LT_CSTR(tst, "abc", "def");
  ASSERT_FALSE(tst, CHECK_LT_CSTR(tst1, "abc", "abc"));
  ASSERT(tst,       CHECK_LT_CSTR(tst1, "abc", "def"));
  ASSERT_FALSE(tst, CHECK_LT_CSTR(tst1, "def", "abc"));
  ASSERT_FALSE(tst, CHECK_LT_CSTR(NULL, "abc", "abc"));
  ASSERT(tst,       CHECK_LT_CSTR(NULL, "abc", "def"));
  ASSERT_FALSE(tst, CHECK_LT_CSTR(NULL, "def", "abc"));


  /* LE <= */
  ASSERT_LE_CSTR(tst, "abc", "def");
  ASSERT_LE_CSTR(tst, "abc", "abc");
  ASSERT(tst,       CHECK_LE_CSTR(tst1, "abc", "abc"));
  ASSERT(tst,       CHECK_LE_CSTR(tst1, "abc", "def"));
  ASSERT_FALSE(tst, CHECK_LE_CSTR(tst1, "def", "abc"));
  ASSERT(tst,       CHECK_LE_CSTR(NULL, "abc", "abc"));
  ASSERT(tst,       CHECK_LE_CSTR(NULL, "abc", "def"));
  ASSERT_FALSE(tst, CHECK_LE_CSTR(NULL, "def", "abc"));

  unit_free_context(ctx);
}

static void test_check_arr_cstr(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  unit_test_t * tst1   = unit_create_test(suite, "test1", test_check_arr_cstr);
  const char * a1[3] = { "abc", "def", "ghi" };
  const char * a2[2] = { "abc", "ghi" };

  /* EQ == */
  ASSERT_EQ_ARR_CSTR(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_EQ_ARR_CSTR(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_CSTR(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_CSTR(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_EQ_ARR_CSTR(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_CSTR(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_EQ_ARR_CSTR(NULL, a2, 2, a1, 3));

  /* NEQ != */
  ASSERT_NEQ_ARR_CSTR(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_CSTR(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_CSTR(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_CSTR(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_NEQ_ARR_CSTR(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_NEQ_ARR_CSTR(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_NEQ_ARR_CSTR(NULL, a2, 2, a1, 3));

  /* GT > */
  ASSERT_GT_ARR_CSTR(tst, a2, 2, a1, 3);
  ASSERT_FALSE(tst, CHECK_GT_ARR_CSTR(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_CSTR(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_CSTR(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_CSTR(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GT_ARR_CSTR(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GT_ARR_CSTR(NULL, a2, 2, a1, 3));

  /* GE >= */
  ASSERT_GE_ARR_CSTR(tst, a2, 2, a1, 3);
  ASSERT_GE_ARR_CSTR(tst, a2, 2, a2, 2);
  ASSERT(tst,       CHECK_GE_ARR_CSTR(tst1, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_CSTR(tst1, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_CSTR(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_GE_ARR_CSTR(NULL, a1, 3, a1, 3));
  ASSERT_FALSE(tst, CHECK_GE_ARR_CSTR(NULL, a1, 3, a2, 2));
  ASSERT(tst,       CHECK_GE_ARR_CSTR(NULL, a2, 2, a1, 3));

     
  /* LT < */
  ASSERT_LT_ARR_CSTR(tst, a1, 3, a2, 2);
  ASSERT_FALSE(tst, CHECK_LT_ARR_CSTR(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_CSTR(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_CSTR(tst1, a2, 2, a1, 3));
  ASSERT_FALSE(tst, CHECK_LT_ARR_CSTR(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LT_ARR_CSTR(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LT_ARR_CSTR(NULL, a2, 2, a1, 3));

  /* LE <= */
  ASSERT_LE_ARR_CSTR(tst, a1, 3, a2, 2);
  ASSERT_LE_ARR_CSTR(tst, a1, 3, a1, 3);
  ASSERT(tst,       CHECK_LE_ARR_CSTR(tst1, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_CSTR(tst1, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_CSTR(tst1, a2, 2, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_CSTR(NULL, a1, 3, a1, 3));
  ASSERT(tst,       CHECK_LE_ARR_CSTR(NULL, a1, 3, a2, 2));
  ASSERT_FALSE(tst, CHECK_LE_ARR_CSTR(NULL, a2, 2, a1, 3));

  unit_free_context(ctx);
}


/*********************************************************************
 * 
 * test framework test
 *
 *********************************************************************/
static void test_suite_with_no_tests(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_create_suite(ctx, "suite2");
  unit_run(stdout, ctx);
  ASSERT_EQ_U(tst, ctx->n_suites_passed,0);
  ASSERT_EQ_U(tst, ctx->n_suites_failed,0);
  ASSERT_EQ_U(tst, ctx->n_suites_without_tests,2);
  ASSERT_EQ_U(tst, ctx->n_tests_passed,0);
  ASSERT_EQ_U(tst, ctx->n_tests_failed,0);
  ASSERT_EQ_U(tst, ctx->n_tests_without_assertions,0);
  ASSERT_EQ_U(tst, ctx->n_assertions_passed,0);
  ASSERT_EQ_U(tst, ctx->n_assertions_failed,0);
  unit_free_context(ctx);
}

static void _test_no_failures(unit_test_t * tst)
{
  ASSERT_EQ_U(tst, 1,1);
  ASSERT(tst, 1);
}

static void _test_with_failures(unit_test_t * tst)
{
  ASSERT_EQ_U(tst, 1,1);
  ASSERT_EQ_U(tst, 0,1);
  /* the following assertion is not checked */
  ASSERT_EQ_U(tst, 1,1);
}

static void _test_with_failed_checks(unit_test_t * tst)
{
  /* does not count */
  int one = 1;
  CHECK_EQ_U(tst, 1,one);
  CHECK(tst, 0);
}

static void _test_no_assertions(unit_test_t * tst)
{

}

static void test_no_failures(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_suite_t * suite = unit_create_suite(ctx, "suite2");
  TEST(suite, _test_no_failures);  
  TEST(suite, _test_no_assertions);  
  unit_run(stdout, ctx);
  ASSERT_EQ_U(tst, ctx->n_suites_passed,1);
  ASSERT_EQ_U(tst, ctx->n_suites_failed,0);
  ASSERT_EQ_U(tst, ctx->n_suites_without_tests,1);

  ASSERT_EQ_U(tst, ctx->n_tests_passed,1);
  ASSERT_EQ_U(tst, ctx->n_tests_failed,0);
  ASSERT_EQ_U(tst, ctx->n_tests_without_assertions,1);

  ASSERT_EQ_U(tst, ctx->n_assertions_passed,2);
  ASSERT_EQ_U(tst, ctx->n_assertions_failed,0);
  unit_free_context(ctx);
}


static void test_with_failures(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_suite_t * suite = unit_create_suite(ctx, "suite2");
  TEST(suite, _test_no_failures);  
  TEST(suite, _test_with_failures);  
  TEST(suite, _test_with_failed_checks);  
  TEST(suite, _test_no_assertions);  
  unit_run(stdout, ctx);
  ASSERT_EQ_U(tst, ctx->n_suites_passed,0);
  ASSERT_EQ_U(tst, ctx->n_suites_failed,1);
  ASSERT_EQ_U(tst, ctx->n_suites_without_tests,1);

  ASSERT_EQ_U(tst, ctx->n_tests_passed,1);
  ASSERT_EQ_U(tst, ctx->n_tests_failed,2);
  ASSERT_EQ_U(tst, ctx->n_tests_without_assertions,1);

  ASSERT_EQ_U(tst, ctx->n_assertions_passed,3);
  ASSERT_EQ_U(tst, ctx->n_assertions_failed,2);
  unit_free_context(ctx);
}

static void test_with_failed_checks(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_suite_t * suite = unit_create_suite(ctx, "suite2");
  TEST(suite, _test_no_failures);  
  TEST(suite, _test_with_failed_checks);  
  TEST(suite, _test_no_assertions);  
  unit_run(stdout, ctx);
  ASSERT_EQ_U(tst, ctx->n_suites_passed,0);
  ASSERT_EQ_U(tst, ctx->n_suites_failed,1);
  ASSERT_EQ_U(tst, ctx->n_suites_without_tests,1);

  ASSERT_EQ_U(tst, ctx->n_tests_passed,1);
  ASSERT_EQ_U(tst, ctx->n_tests_failed,1);
  ASSERT_EQ_U(tst, ctx->n_tests_without_assertions,1);

  ASSERT_EQ_U(tst, ctx->n_assertions_passed,2);
  ASSERT_EQ_U(tst, ctx->n_assertions_failed,1);
  unit_free_context(ctx);
}

void test_test(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "test");
  TEST(suite, test_check_boolean);
  TEST(suite, test_check_int);
  TEST(suite, test_check_arr_int);
  TEST(suite, test_check_unsigned_int);
  TEST(suite, test_check_arr_unsigned_int);
  TEST(suite, test_check_ptr);
  TEST(suite, test_check_arr_ptr);
  TEST(suite, test_check_cstr);
  TEST(suite, test_check_arr_cstr);

  TEST(suite, test_suite_with_no_tests);
  TEST(suite, test_no_failures);
  TEST(suite, test_with_failures);
  TEST(suite, test_with_failed_checks);
}
