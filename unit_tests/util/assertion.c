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
#include "lisp/util/unit_test.h"
#include "lisp/util/xmalloc.h"
#include "lisp/util/assertion.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static int _signal_handler(assertion_t * assertion, void * user_data)
{
  int * d  = (int*)user_data;
  *d = 1;
  return 1;
}

static int _signal_cancel_handler(assertion_t * assertion, void * user_data)
{
  int * d  = (int*)user_data;
  *d = 1;
  return 0;
}

static void test_create_assertion(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create("myfile.c", 27);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT_FALSE(tst, assertion->success);
  ASSERT_FALSE(tst, assertion->is_exception);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_invert(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_invert(assertion_create("myfile.c", 27));
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT(tst, assertion->success);
  ASSERT_FALSE(tst, assertion->is_exception);

  assertion = assertion_invert(assertion);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT_FALSE(tst, assertion->success);
  ASSERT_FALSE(tst, assertion->is_exception);

  assertion->is_exception = 1;
  assertion = assertion_invert(assertion);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT(tst, assertion->success);
  ASSERT(tst, assertion->is_exception);

  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


/*********************************************************************
 * 
 * assertion helpers
 *
 *********************************************************************/
static int success(assertion_t * assertion, int expected)
{
  if(assertion == NULL) 
  {
    if(expected) 
    {
      return 1;
    }
    else 
    {
      return 0;
    }
  }
  else 
  {
    int ret = assertion->success;
    assertion_free(assertion);
    if(expected) 
    {
      return ret;
    }
    else 
    {
      return !ret;
    }
  }
}

#define ASSERTS_CMP(__CMP_TYPE__, __A__, __B__,__OP__,__EX__)	\
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_##__CMP_TYPE__("f", 0,		\
						     #__A__,		\
						     #__B__,		\
						     (__A__),		\
						     (__B__),		\
						     (__OP__),		\
						     1),		\
		 (__EX__)));						\
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_##__CMP_TYPE__("f", 0,		\
						     #__A__,		\
						     #__B__,		\
						     (__A__),		\
						     (__B__),		\
						     (__OP__),		\
						     0),		\
		 (__EX__)));					      
  
#define ASSERTS_CMP_ARR(__CMP_TYPE__, __A__, __NA__,__B__,__NB__,__OP__,__EX__) \
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_arr_##__CMP_TYPE__("f", 0,	\
							 #__A__,	\
							 #__B__,	\
							 (__A__),	\
							 (__NA__),	\
							 (__B__),	\
							 (__NB__),	\
							 (__OP__),	\
							 1),		\
		 (__EX__)));						\
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_arr_##__CMP_TYPE__("f", 0,	\
							 #__A__,	\
							 #__B__,	\
							 (__A__),	\
							 (__NA__),	\
							 (__B__),	\
							 (__NB__),	\
							 (__OP__),	\
							 0),		\
		 (__EX__)));

#define ASSERT_SIGNAL(__TEST__, __EXPR__)			\
  {								\
    assertion_handler_t my_handler;				\
    assertion_handler_t old_handler;				\
    int signal_captured = 0;					\
    my_handler.handler_cb  = _signal_cancel_handler;		\
    my_handler.user_data   = &signal_captured;			\
    my_handler.fp          = NULL;				\
    my_handler.use_stderr  = 0;					\
    old_handler = assertion_register_handler(my_handler);	\
    __EXPR__;							\
    assertion_register_handler(old_handler);			\
    ASSERT(__TEST__, signal_captured);				\
  }


/*********************************************************************
 * 
 * test boolean
 *
 *********************************************************************/
static void test_assertion_create_true(unit_test_t * tst)
{
  ASSERT(tst, success(assertion_create_true(__FILE__,
					    __LINE__,
					    "a",
					    1,
					    0), 1));
  ASSERT(tst, success(assertion_create_true(__FILE__,
					    __LINE__,
					    "a",
					    1,
					    1), 1));
  ASSERT(tst, success(assertion_create_true(__FILE__,
					    __LINE__,
					    "a",
					    0,
					    1), 0));
  ASSERT(tst, success(assertion_create_true(__FILE__,
					    __LINE__,
					    "a",
					    0,
					    0), 0));
}

static void test_assertion_create_false(unit_test_t * tst)
{
  ASSERT(tst, success(assertion_create_false(__FILE__,
					     __LINE__,
					     "a",
					     0,
					     0), 1));
  ASSERT(tst, success(assertion_create_false(__FILE__,
					    __LINE__,
					    "a",
					    0,
					    1), 1));
  ASSERT(tst, success(assertion_create_false(__FILE__,
					    __LINE__,
					    "a",
					    1,
					    1), 0));
  ASSERT(tst, success(assertion_create_false(__FILE__,
					    __LINE__,
					    "a",
					    1,
					    0), 0));
}

static void test_assertion_raise_boolean(unit_test_t * tst)
{
  REQUIRE(1);
  REQUIRE_FALSE(0);
  ASSERT_SIGNAL(tst, REQUIRE(0));
  ASSERT_SIGNAL(tst, REQUIRE_FALSE(1));
}

/*********************************************************************
 * 
 * int test
 *
 *********************************************************************/
static void test_cmp_i(unit_test_t * tst)
{
  ASSERT_LT_I(tst, assertion_cmp_i(-1,  2), 0);
  ASSERT_GT_I(tst, assertion_cmp_i(2,  -1), 0);
  ASSERT_EQ_I(tst, assertion_cmp_i(-1, -1), 0);
}

static void test_assertion_create_cmp_i(unit_test_t * tst)
{
  int a = -1;
  int b =  2;
  ASSERTS_CMP(i, a, a, "==", 1);
  ASSERTS_CMP(i, a, a, "<=", 1);
  ASSERTS_CMP(i, a, a, "<",  0);
  ASSERTS_CMP(i, a, a, ">=", 1);
  ASSERTS_CMP(i, a, a, ">",  0);

  ASSERTS_CMP(i, a, b, "==", 0);
  ASSERTS_CMP(i, a, b, "<=", 1);
  ASSERTS_CMP(i, a, b, "<",  1);
  ASSERTS_CMP(i, a, b, ">=", 0);
  ASSERTS_CMP(i, a, b, ">",  0);

  ASSERTS_CMP(i, b, a, "==", 0);
  ASSERTS_CMP(i, b, a, "<=", 0);
  ASSERTS_CMP(i, b, a, "<",  0);
  ASSERTS_CMP(i, b, a, ">=", 1);
  ASSERTS_CMP(i, b, a, ">",  1);
}


static void test_assertion_raise_cmp_i(unit_test_t * tst)
{
  int a = -1;
  int b = 2;
  /* EQ == */
  REQUIRE_EQ_I(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_EQ_I(a,  b));
  ASSERT_SIGNAL(tst, REQUIRE_EQ_I(b,  a));

  /* NEQ != */
  ASSERT_SIGNAL(tst, REQUIRE_NEQ_I(a, a));
  REQUIRE_NEQ_I(a, b);
  REQUIRE_NEQ_I(b, a);

  /* GT > */
  ASSERT_SIGNAL(tst, REQUIRE_GT_I(a, a));
  ASSERT_SIGNAL(tst, REQUIRE_GT_I(a, b));
  REQUIRE_GT_I(b, a);

  /* GE >= */
  REQUIRE_GE_I(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_GE_I(a, b));
  REQUIRE_GE_I(b, a);
     
  /* LT < */
  ASSERT_SIGNAL(tst, REQUIRE_LT_I(a, a));
  REQUIRE_LT_I(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LT_I(b, a));

  /* LE <= */
  REQUIRE_LE_I(a, a);
  REQUIRE_LE_I(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LE_I(b, a));
}

/*********************************************************************
 * 
 * vecotr of int
 *
 *********************************************************************/
static void test_cmp_arr_i(unit_test_t * tst)
{
  const int a1[3] = { 1, 2, 3};
  const int a2[3] = { 1, 2, 4};
  const int a3[3] = { 4, 5, 6};

  ASSERT_LT_I(tst, assertion_cmp_arr_i(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_i(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_i(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_i(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_i(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_i(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_i(a1, 3, a1, 3), 0);
}

static void test_assertion_create_cmp_arr_i(unit_test_t * tst)
{
  const int a[3] = { 1, 2, 3};
  const int b[3] = { 4, 5, 6};
  ASSERTS_CMP_ARR(i, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(i, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(i, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, ">",  1);
}


static void test_assertion_raise_cmp_arr_i(unit_test_t * tst)
{
  const int a[3] = { 1, 2, 3};
  const int b[3] = { 4, 5, 6};
  REQUIRE_EQ_ARR_I(a,3, a, 3);
  REQUIRE_LE_ARR_I(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_LT_ARR_I(a,3, a, 3));
  REQUIRE_GE_ARR_I(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_GT_ARR_I(a,3, a, 3));

  ASSERT_SIGNAL(tst,REQUIRE_EQ_ARR_I(a, 3, b, 3));
  REQUIRE_LE_ARR_I(a, 3, b, 3);
  REQUIRE_LT_ARR_I(a, 3, b, 3);
  ASSERT_SIGNAL(tst,REQUIRE_GE_ARR_I(a, 3, b, 3));
  ASSERT_SIGNAL(tst,REQUIRE_GT_ARR_I(a, 3, b, 3));
  

  ASSERT_SIGNAL(tst, REQUIRE_EQ_ARR_I(b, 3, a, 3));
  ASSERT_SIGNAL(tst, REQUIRE_LE_ARR_I(b, 3, a, 3));
  ASSERT_SIGNAL(tst, REQUIRE_LT_ARR_I(b, 3, a, 3));
  REQUIRE_GE_ARR_I(b, 3, a, 3);
  REQUIRE_GT_ARR_I(b, 3, a, 3);
}


/*********************************************************************
 * 
 * unsigned int 
 *
 *********************************************************************/
static void test_cmp_u(unit_test_t * tst)
{
  ASSERT_LT_I(tst, assertion_cmp_u(1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_u(2, 1), 0);
  ASSERT_EQ_I(tst, assertion_cmp_u(1, 1), 0);
}

static void test_assertion_create_cmp_u(unit_test_t * tst)
{
  unsigned int a =  1;
  unsigned int b =  2;
  ASSERTS_CMP(u, a, a, "==", 1);
  ASSERTS_CMP(u, a, a, "<=", 1);
  ASSERTS_CMP(u, a, a, "<",  0);
  ASSERTS_CMP(u, a, a, ">=", 1);
  ASSERTS_CMP(u, a, a, ">",  0);

  ASSERTS_CMP(u, a, b, "==", 0);
  ASSERTS_CMP(u, a, b, "<=", 1);
  ASSERTS_CMP(u, a, b, "<",  1);
  ASSERTS_CMP(u, a, b, ">=", 0);
  ASSERTS_CMP(u, a, b, ">",  0);

  ASSERTS_CMP(u, b, a, "==", 0);
  ASSERTS_CMP(u, b, a, "<=", 0);
  ASSERTS_CMP(u, b, a, "<",  0);
  ASSERTS_CMP(u, b, a, ">=", 1);
  ASSERTS_CMP(u, b, a, ">",  1);
}

static void test_assertion_raise_cmp_u(unit_test_t * tst)
{
  int a = 1;
  int b = 2;
  /* EQ == */
  REQUIRE_EQ_U(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_EQ_U(a,  b));
  ASSERT_SIGNAL(tst, REQUIRE_EQ_U(b,  a));

  /* NEQ != */
  ASSERT_SIGNAL(tst, REQUIRE_NEQ_U(a, a));
  REQUIRE_NEQ_U(a, b);
  REQUIRE_NEQ_U(b, a);

  /* GT > */
  ASSERT_SIGNAL(tst, REQUIRE_GT_U(a, a));
  ASSERT_SIGNAL(tst, REQUIRE_GT_U(a, b));
  REQUIRE_GT_U(b, a);

  /* GE >= */
  REQUIRE_GE_U(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_GE_U(a, b));
  REQUIRE_GE_U(b, a);
     
  /* LT < */
  ASSERT_SIGNAL(tst, REQUIRE_LT_U(a, a));
  REQUIRE_LT_U(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LT_U(b, a));

  /* LE <= */
  REQUIRE_LE_U(a, a);
  REQUIRE_LE_U(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LE_U(b, a));
}

/*********************************************************************
 * 
 * vector of unsigned int 
 *
 *********************************************************************/
static void test_cmp_arr_u(unit_test_t * tst)
{
  const unsigned a1[3] = { 1, 2, 3};
  const unsigned a2[3] = { 1, 2, 4};
  const unsigned a3[3] = { 4, 5, 6};

  ASSERT_LT_I(tst, assertion_cmp_arr_u(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_u(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_u(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_u(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_u(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_u(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_u(a1, 3, a1, 3), 0);
}

static void test_assertion_create_cmp_arr_u(unit_test_t * tst)
{
  const unsigned int a[3] = { 1, 2, 3};
  const unsigned int b[3] = { 4, 5, 6};

  ASSERTS_CMP_ARR(u, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(u, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(u, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, ">",  1);
}


static void test_assertion_raise_cmp_arr_u(unit_test_t * tst)
{
  const unsigned int a[3] = { 1, 2, 3};
  const unsigned int b[3] = { 4, 5, 6};
  REQUIRE_EQ_ARR_U(a,3, a, 3);
  REQUIRE_LE_ARR_U(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_LT_ARR_U(a,3, a, 3));
  REQUIRE_GE_ARR_U(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_GT_ARR_U(a,3, a, 3));

  ASSERT_SIGNAL(tst,REQUIRE_EQ_ARR_U(a, 3, b, 3));
  REQUIRE_LE_ARR_U(a, 3, b, 3);
  REQUIRE_LT_ARR_U(a, 3, b, 3);
  ASSERT_SIGNAL(tst,REQUIRE_GE_ARR_U(a, 3, b, 3));
  ASSERT_SIGNAL(tst,REQUIRE_GT_ARR_U(a, 3, b, 3));
  

  ASSERT_SIGNAL(tst, REQUIRE_EQ_ARR_U(b, 3, a, 3));
  ASSERT_SIGNAL(tst, REQUIRE_LE_ARR_U(b, 3, a, 3));
  ASSERT_SIGNAL(tst, REQUIRE_LT_ARR_U(b, 3, a, 3));
  REQUIRE_GE_ARR_U(b, 3, a, 3);
  REQUIRE_GT_ARR_U(b, 3, a, 3);
}

/*********************************************************************
 * 
 * pointer
 *
 *********************************************************************/
static void test_cmp_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  ASSERT_LT_I(tst, assertion_cmp_ptr(arr + 1, arr + 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_ptr(arr + 2, arr + 1), 0);
  ASSERT_EQ_I(tst, assertion_cmp_ptr(arr + 1, arr + 1), 0);
  FREE(arr);
}

static void test_assertion_create_cmp_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a = arr + 1;
  const void * b = arr + 2;

  ASSERTS_CMP(ptr, a, a, "==", 1);
  ASSERTS_CMP(ptr, a, a, "<=", 1);
  ASSERTS_CMP(ptr, a, a, "<",  0);
  ASSERTS_CMP(ptr, a, a, ">=", 1);
  ASSERTS_CMP(ptr, a, a, ">",  0);

  ASSERTS_CMP(ptr, a, b, "==", 0);
  ASSERTS_CMP(ptr, a, b, "<=", 1);
  ASSERTS_CMP(ptr, a, b, "<",  1);
  ASSERTS_CMP(ptr, a, b, ">=", 0);
  ASSERTS_CMP(ptr, a, b, ">",  0);

  ASSERTS_CMP(ptr, b, a, "==", 0);
  ASSERTS_CMP(ptr, b, a, "<=", 0);
  ASSERTS_CMP(ptr, b, a, "<",  0);
  ASSERTS_CMP(ptr, b, a, ">=", 1);
  ASSERTS_CMP(ptr, b, a, ">",  1);

  FREE(arr);
}

static void test_assertion_raise_cmp_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a = arr + 1;
  const void * b = arr + 2;
  /* EQ == */
  REQUIRE_EQ_PTR(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_EQ_PTR(a,  b));
  ASSERT_SIGNAL(tst, REQUIRE_EQ_PTR(b,  a));

  /* NEQ != */
  ASSERT_SIGNAL(tst, REQUIRE_NEQ_PTR(a, a));
  REQUIRE_NEQ_PTR(a, b);
  REQUIRE_NEQ_PTR(b, a);

  /* GT > */
  ASSERT_SIGNAL(tst, REQUIRE_GT_PTR(a, a));
  ASSERT_SIGNAL(tst, REQUIRE_GT_PTR(a, b));
  REQUIRE_GT_PTR(b, a);

  /* GE >= */
  REQUIRE_GE_PTR(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_GE_PTR(a, b));
  REQUIRE_GE_PTR(b, a);
     
  /* LT < */
  ASSERT_SIGNAL(tst, REQUIRE_LT_PTR(a, a));
  REQUIRE_LT_PTR(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LT_PTR(b, a));

  /* LE <= */
  REQUIRE_LE_PTR(a, a);
  REQUIRE_LE_PTR(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LE_PTR(b, a));
  FREE(arr);
}

/*********************************************************************
 * 
 * vector of pointer
 *
 *********************************************************************/
static void test_cmp_arr_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a1[3] = { arr + 1, arr + 2, arr + 3};
  const void * a2[3] = { arr + 1, arr + 2, arr + 4};
  const void * a3[3] = { arr + 4, arr + 5, arr + 6};

  ASSERT_LT_I(tst, assertion_cmp_arr_ptr(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_ptr(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_ptr(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_ptr(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_ptr(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_ptr(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_ptr(a1, 3, a1, 3), 0);

  FREE(arr);
}

static void test_assertion_create_cmp_arr_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a[3] = { arr + 1, arr + 2, arr + 3};
  const void * b[3] = { arr + 4, arr + 5, arr + 6};

  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, ">",  1);

  FREE(arr);
}

static void test_assertion_raise_cmp_arr_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a[3] = { arr + 1, arr + 2, arr + 3};
  const void * b[3] = { arr + 4, arr + 5, arr + 6};
  REQUIRE_EQ_ARR_PTR(a,3, a, 3);
  REQUIRE_LE_ARR_PTR(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_LT_ARR_PTR(a,3, a, 3));
  REQUIRE_GE_ARR_PTR(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_GT_ARR_PTR(a,3, a, 3));

  ASSERT_SIGNAL(tst,REQUIRE_EQ_ARR_PTR(a, 3, b, 3));
  REQUIRE_LE_ARR_PTR(a, 3, b, 3);
  REQUIRE_LT_ARR_PTR(a, 3, b, 3);
  ASSERT_SIGNAL(tst,REQUIRE_GE_ARR_PTR(a, 3, b, 3));
  ASSERT_SIGNAL(tst,REQUIRE_GT_ARR_PTR(a, 3, b, 3));

  FREE(arr);
}

/*********************************************************************
 * 
 * c-string
 *
 *********************************************************************/
static void test_cmp_cstr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  ASSERT_LT_I(tst, assertion_cmp_cstr("abc", "def"), 0);
  ASSERT_GT_I(tst, assertion_cmp_cstr("def", "abc"), 0);
  ASSERT_EQ_I(tst, assertion_cmp_cstr("abc", "abc"), 0);
  FREE(arr);
}

static void test_assertion_create_cmp_cstr(unit_test_t * tst)
{
  const char * a = "abc";
  const char * b = "def";
  ASSERTS_CMP(cstr, a, a, "==", 1);
  ASSERTS_CMP(cstr, a, a, "<=", 1);
  ASSERTS_CMP(cstr, a, a, "<",  0);
  ASSERTS_CMP(cstr, a, a, ">=", 1);
  ASSERTS_CMP(cstr, a, a, ">",  0);

  ASSERTS_CMP(cstr, a, b, "==", 0);
  ASSERTS_CMP(cstr, a, b, "<=", 1);
  ASSERTS_CMP(cstr, a, b, "<",  1);
  ASSERTS_CMP(cstr, a, b, ">=", 0);
  ASSERTS_CMP(cstr, a, b, ">",  0);

  ASSERTS_CMP(cstr, b, a, "==", 0);
  ASSERTS_CMP(cstr, b, a, "<=", 0);
  ASSERTS_CMP(cstr, b, a, "<",  0);
  ASSERTS_CMP(cstr, b, a, ">=", 1);
  ASSERTS_CMP(cstr, b, a, ">",  1);
}

static void test_assertion_raise_cmp_cstr(unit_test_t * tst)
{
  const char * a = "abc";
  const char * b = "def";
  /* EQ == */
  REQUIRE_EQ_CSTR(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_EQ_CSTR(a,  b));
  ASSERT_SIGNAL(tst, REQUIRE_EQ_CSTR(b,  a));

  /* NEQ != */
  ASSERT_SIGNAL(tst, REQUIRE_NEQ_CSTR(a, a));
  REQUIRE_NEQ_CSTR(a, b);
  REQUIRE_NEQ_CSTR(b, a);

  /* GT > */
  ASSERT_SIGNAL(tst, REQUIRE_GT_CSTR(a, a));
  ASSERT_SIGNAL(tst, REQUIRE_GT_CSTR(a, b));
  REQUIRE_GT_CSTR(b, a);

  /* GE >= */
  REQUIRE_GE_CSTR(a, a);
  ASSERT_SIGNAL(tst, REQUIRE_GE_CSTR(a, b));
  REQUIRE_GE_CSTR(b, a);
     
  /* LT < */
  ASSERT_SIGNAL(tst, REQUIRE_LT_CSTR(a, a));
  REQUIRE_LT_CSTR(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LT_CSTR(b, a));

  /* LE <= */
  REQUIRE_LE_CSTR(a, a);
  REQUIRE_LE_CSTR(a, b);
  ASSERT_SIGNAL(tst, REQUIRE_LE_CSTR(b, a));
}

/*********************************************************************
 * 
 * vector of c-string
 *
 *********************************************************************/
static void test_cmp_arr_cstr(unit_test_t * tst)
{
  const char * a1[3] = { "1", "2", "3"};
  const char * a2[3] = { "1", "2", "4"};
  const char * a3[3] = { "4", "5", "6"};

  ASSERT_LT_I(tst, assertion_cmp_arr_cstr(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_cstr(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_cstr(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_cstr(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_cstr(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_cstr(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_cstr(a1, 3, a1, 3), 0);
}

static void test_assertion_create_cmp_arr_cstr(unit_test_t * tst)
{
  const char * a[3] = { "1", "2", "3"};
  const char * b[3] = { "4", "5", "6"};

  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, ">",  1);
}

static void test_assertion_raise_cmp_arr_cstr(unit_test_t * tst)
{
  const char * a[3] = { "1", "2", "3"};
  const char * b[3] = { "4", "5", "6"};
  REQUIRE_EQ_ARR_CSTR(a,3, a, 3);
  REQUIRE_LE_ARR_CSTR(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_LT_ARR_CSTR(a,3, a, 3));
  REQUIRE_GE_ARR_CSTR(a,3, a, 3);
  ASSERT_SIGNAL(tst, REQUIRE_GT_ARR_CSTR(a,3, a, 3));

  ASSERT_SIGNAL(tst,REQUIRE_EQ_ARR_CSTR(a, 3, b, 3));
  REQUIRE_LE_ARR_CSTR(a, 3, b, 3);
  REQUIRE_LT_ARR_CSTR(a, 3, b, 3);
  ASSERT_SIGNAL(tst,REQUIRE_GE_ARR_CSTR(a, 3, b, 3));
  ASSERT_SIGNAL(tst,REQUIRE_GT_ARR_CSTR(a, 3, b, 3));
}

/*********************************************************************
 * 
 * print
 *
 *********************************************************************/
#ifdef HAS_FMEMOPEN
#define COLOR_RED      "\x1B[31m"
#define COLOR_NORMAL   "\x1B[0m"
#define COLOR_GREEN    "\x1B[32m"
FILE *fmemopen (void *__s, size_t __len, const char *__modes);
static void test_assertion_print_exception(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_message("myfile.c", 27, "EXPECT","EXPLAIN\nLINE2", 1,0);
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  FILE * fp;
  fp = fmemopen(buffer, 1024, "w");
  assertion_print(fp, assertion, 0);
  fclose(fp);
  ASSERT_EQ_CSTR(tst,
		 "[FAILED]    myfile.c:27: exception <EXPECT>\n"
		 "                         EXPLAIN\n"
		 "                         LINE2\n",
		 buffer);
  FREE(buffer);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_print_assertion_cmp_i(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_cmp_i("myfile.c", 
						   27,
						   "a",
						   "b",
						   1,
						   2,
						   "==",
						   0);
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  FILE * fp;
  fp = fmemopen(buffer, 1024, "w");
  assertion_print(fp, assertion, 1);
  fclose(fp);
  ASSERT_EQ_CSTR(tst,
                 COLOR_RED"[FAILED]"COLOR_NORMAL
		   "    myfile.c:27: assertion <a==b>\n"
		   "                         with a==1\n"
		   "                              b==2\n",
		 buffer);
  FREE(buffer);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_print_assertion_cmp_u(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_cmp_u("myfile.c", 
						   27,
						   "a",
						   "2",
						   1,
						   2,
						   "!=",
						   0);
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  FILE * fp;
  fp = fmemopen(buffer, 1024, "w");
  assertion_print(fp, assertion, 1);
  fclose(fp);
  ASSERT_EQ_CSTR(tst,
                 COLOR_GREEN"[PASSED]"COLOR_NORMAL
		   "    myfile.c:27: assertion <a!=2>\n"
		   "                         with a==1\n",
		 buffer);
  FREE(buffer);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

#endif

static void test_assertion_dont_raise(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_message("myfile.c", 27, NULL,NULL, 1, 1);
  assertion_raise(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_raise(unit_test_t * tst)
{
  memcheck_begin();
  int                 signal_captured = 0;
  assertion_handler_t my_handler;
  my_handler.handler_cb  = _signal_handler;
  my_handler.user_data   = &signal_captured;
  my_handler.fp          = NULL;
  my_handler.use_stderr  = 0;
#ifdef HAS_FMEMOPEN
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  my_handler.fp = fmemopen(buffer, 1024, "w");
#endif
  void (*prev_handler)(int);
  assertion_handler_t old_handler;
  assertion_t * assertion;
  old_handler = assertion_register_handler(my_handler);
  assertion = assertion_create_message("myfile.c", 27, NULL,NULL, 0,0);
  signal_captured = 0;
  prev_handler = signal (SIGABRT, SIG_IGN); 
  assertion_raise(assertion);
  signal (SIGABRT, prev_handler);
  assertion_register_handler(old_handler);
  ASSERT(tst, signal_captured);
#ifdef HAS_FMEMOPEN
  fclose(my_handler.fp);
  ASSERT_GT_U(tst, strlen(buffer),0);
  FREE(buffer);
#endif
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_assertion(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "assertion");
  TEST(suite, test_create_assertion);
  TEST(suite, test_assertion_invert);

  TEST(suite, test_assertion_create_true);
  TEST(suite, test_assertion_create_false);
  TEST(suite, test_assertion_raise_boolean);

  TEST(suite, test_cmp_i);
  TEST(suite, test_assertion_create_cmp_i);
  TEST(suite, test_assertion_raise_cmp_i);

  TEST(suite, test_cmp_arr_i);
  TEST(suite, test_assertion_create_cmp_arr_i);
  TEST(suite, test_assertion_raise_cmp_arr_i);

  TEST(suite, test_cmp_u);
  TEST(suite, test_assertion_create_cmp_u);
  TEST(suite, test_assertion_raise_cmp_u);

  TEST(suite, test_cmp_arr_u);
  TEST(suite, test_assertion_create_cmp_arr_u);
  TEST(suite, test_assertion_raise_cmp_arr_u);

  TEST(suite, test_cmp_ptr);
  TEST(suite, test_assertion_create_cmp_ptr);
  TEST(suite, test_assertion_raise_cmp_ptr);

  TEST(suite, test_cmp_arr_ptr);
  TEST(suite, test_assertion_create_cmp_arr_ptr);
  TEST(suite, test_assertion_raise_cmp_arr_ptr);

  TEST(suite, test_cmp_cstr);
  TEST(suite, test_assertion_create_cmp_cstr);
  TEST(suite, test_assertion_raise_cmp_cstr);

  TEST(suite, test_cmp_arr_cstr);
  TEST(suite, test_assertion_create_cmp_arr_cstr);
  TEST(suite, test_assertion_raise_cmp_arr_cstr);

#ifdef HAS_FMEMOPEN
  TEST(suite, test_assertion_print_exception);
  TEST(suite, test_assertion_print_assertion_cmp_i);
  TEST(suite, test_assertion_print_assertion_cmp_u);
#endif
  TEST(suite, test_assertion_dont_raise);
  TEST(suite, test_assertion_raise);
}
