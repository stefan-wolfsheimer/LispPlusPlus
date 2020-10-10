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
#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__
#include <stdlib.h>
#include <stdio.h>
#include "assertion.h"

/* @todo printer for array assertions */
/* @todo extra library for c string */
/* @todo extra library for assertions */

#define UNIT_ARGV_RUN   1
#define UNIT_ARGV_HELP  2
#define UNIT_ARGV_ERROR 3

struct memchecker_t;
struct unit_test_t;
typedef void(*unit_test_function_t)(struct unit_test_t * tst);

typedef struct unit_test_t
{
  char                    * name;
  unit_test_function_t      func;
  struct unit_test_t      * next;
  struct assertion_t      * first_assertion;
  struct assertion_t      * last_assertion;
  struct unit_suite_t     * suite;
  short int                 active;
  void                    * user_data;
  FILE                    * _fp;
  int                       _passed_assertions;
  int                       _failed_assertions;
} unit_test_t;

typedef struct unit_suite_t
{
  char                  * name;
  struct unit_test_t    * first_test;
  struct unit_test_t    * last_test;
  struct unit_context_t * ctx;
  struct unit_suite_t   * next;
  short int               active;
} unit_suite_t;

typedef struct unit_context_t
{
  int verbose_level;
  int color;
  struct unit_suite_t * first_suite;
  struct unit_suite_t * last_suite;
  size_t n_suites_passed;
  size_t n_suites_without_tests;
  size_t n_suites_failed;
  size_t n_suites_deactivated;
  size_t n_tests_passed;
  size_t n_tests_without_assertions;
  size_t n_tests_failed;
  size_t n_tests_deactivated;
  size_t n_assertions_passed;
  size_t n_assertions_failed;
} unit_context_t;

unit_context_t * unit_create_context();

void unit_free_context(unit_context_t * ctx);

int unit_parse_argv(unit_context_t * ctx, int argc, const char ** argv);

void unit_print_help(FILE * fp, unit_context_t * ctx, const char * progr);

unit_suite_t * unit_create_suite(unit_context_t * ctx, 
                                 const char * name);

unit_test_t * unit_create_test(unit_suite_t * suite, 
                               const char * name, 
                               unit_test_function_t func);

/** 
 *  Add a list of assertion (or a single assertion to the test).
 *  @return non zero if all assertions are successful or assertion_lst is NULL
 */
int unit_add_assertion(unit_test_t * tst,
		       assertion_t * assertion_lst);

/*********************************************************************
 * 
 * macros to create check and assertion function declarations and
 * assertion macros
 *
 *********************************************************************/
#define __CREATE_ASSERT_CMP__(__TYPE__,__TEST__, __LHS__, __RHS__,_OP_)	\
  if(!unit_add_assertion((__TEST__),					\
			 assertion_create_cmp_##__TYPE__(__FILE__,	\
							 __LINE__,	\
							 (#__LHS__),	\
							 (#__RHS__),	\
							 (__LHS__),	\
							 (__RHS__),	\
							 (_OP_),	\
							 0)))		\
  { return; }

#define __CREATE_ASSERT_ARR_CMP__(__TYPE__,__TEST__,			\
				  __LHS__,__NLHS__,			\
				  __RHS__,__NRHS__,_OP_)		\
  if(!unit_add_assertion((__TEST__),					\
			 assertion_create_cmp_arr_##__TYPE__(__FILE__,	\
							     __LINE__,	\
							     (#__LHS__), \
							     (#__RHS__),\
							     (__LHS__), \
							     (__NLHS__),\
							     (__RHS__), \
							     (__NRHS__),\
							     _OP_,	\
							     0)))	\
			{ return ; }

#define __CREATE_CHECK_CMP__(__TYPE__,__TEST__, __LHS__, __RHS__,_OP_)	\
  unit_add_assertion((__TEST__),					\
		     assertion_create_cmp_##__TYPE__(__FILE__,		\
						     __LINE__,		\
						     (#__LHS__),	\
						     (#__RHS__),	\
						     (__LHS__),		\
						     (__RHS__),		\
						     (_OP_),		\
						     1))

#define __CREATE_CHECK_ARR_CMP__(__TYPE__,__TEST__,			\
				 __LHS__,__NLHS__,__RHS__,__NRHS__,	\
				 __OP__)				\
  unit_add_assertion((__TEST__),					\
		     assertion_create_cmp_arr_##__TYPE__(__FILE__,	\
							 __LINE__,	\
							 (#__LHS__),	\
							 (#__RHS__),	\
							 (__LHS__),	\
							 (__NLHS__),	\
							 (__RHS__),	\
							 (__NRHS__),	\
							 (__OP__),	\
							 1))
  

/*********************************************************************
 * 
 * boolean
 *
 **********************************************************************/
#define ASSERT(__TEST__,__EXPR__)                                       \
  if(!unit_add_assertion((__TEST__),					\
			 assertion_create_true(__FILE__, __LINE__,	\
					       #__EXPR__,(__EXPR__),	\
					       0)))			\
  { return; }

#define ASSERT_FALSE(__TEST__,__EXPR__)					\
  if(!unit_add_assertion((__TEST__),					\
			 assertion_create_false(__FILE__, __LINE__,	\
					       #__EXPR__,(__EXPR__),	\
					       0)))			\
  { return; }

#define CHECK(__TEST__, __EXPR__)                                       \
  unit_add_assertion((__TEST__),					\
		     assertion_create_true(__FILE__, __LINE__,		\
					   #__EXPR__,(__EXPR__),	\
					   1))


#define CHECK_FALSE(__TEST__, __EXPR__)					\
  unit_add_assertion((__TEST__),					\
		     assertion_create_false(__FILE__, __LINE__,		\
					    #__EXPR__,(__EXPR__),	\
					    1))

/*********************************************************************
 * 
 * int
 *
 **********************************************************************/
#define ASSERT_EQ_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(i, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(i, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_I(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(i, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(i, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(i, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(i, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(i, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(i, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(i, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(i, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(i, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(i, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of int
 *
 **********************************************************************/
#define ASSERT_EQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")


#define CHECK_EQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">=")

  

/*********************************************************************
 * 
 * unsigned int
 *
 **********************************************************************/
#define ASSERT_EQ_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(u, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(u, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_U(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(u, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(u, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(u, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(u, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(u, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(u, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(u, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(u, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(u, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(u, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector unsigned int
 *
 **********************************************************************/
#define ASSERT_EQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")


#define CHECK_EQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(u, (__TEST__),				\
				       (__LHS__),(__NLHS__),		\
			   (__RHS__), (__NRHS__), ">=")


/*********************************************************************
 * 
 * pointer
 *
 *********************************************************************/
#define ASSERT_EQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(ptr,                                            \
                        (__TEST__),                                     \
                        (const void*)(__LHS__) ,                        \
                        (const void*)(__RHS__),                         \
                        "==")

#define CHECK_EQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(ptr,                                             \
                       (__TEST__),                                      \
                       (const void*)(__LHS__) ,                         \
                       (const void*)(__RHS__),                          \
                       "==")

#define ASSERT_NEQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(ptr,                                            \
                        (__TEST__),                                     \
                        (const void*)(__LHS__) ,                        \
                        (const void*)(__RHS__),                         \
                        "!=")

#define CHECK_NEQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(ptr,                                             \
                       (__TEST__),                                      \
                       (const void*)(__LHS__) ,                         \
                       (const void*)(__RHS__),                          \
                       "!=")

#define ASSERT_GT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(ptr,                                            \
                        (__TEST__),                                     \
                        (const void*)(__LHS__) ,                        \
                        (const void*)(__RHS__),                         \
                        ">")

#define CHECK_GT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(ptr,                                             \
                       (__TEST__),                                      \
                       (const void*)(__LHS__) ,                         \
                       (const void*)(__RHS__),                          \
                       ">")

#define ASSERT_GE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(ptr,                                            \
                        (__TEST__),                                     \
                        (const void*)(__LHS__) ,                        \
                        (const void*)(__RHS__),                         \
                        ">=")

#define CHECK_GE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(ptr,                                             \
                       (__TEST__),                                      \
                       (const void*)(__LHS__) ,                         \
                       (const void*)(__RHS__),                          \
                       ">=")

#define ASSERT_LT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(ptr,                                            \
                        (__TEST__),                                     \
                        (const void*)(__LHS__) ,                        \
                        (const void*)(__RHS__),                         \
                        "<")

#define CHECK_LT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(ptr,                                             \
                       (__TEST__),                                      \
                       (const void*)(__LHS__) ,                         \
                       (const void*)(__RHS__),                          \
                       "<")

#define ASSERT_LE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(ptr,                                            \
                        (__TEST__),                                     \
                        (const void*)(__LHS__) ,                        \
                        (const void*)(__RHS__),                         \
                        "<=")

#define CHECK_LE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(ptr,                                             \
                       (__TEST__),                                      \
                       (const void*)(__LHS__) ,                         \
                       (const void*)(__RHS__),                          \
                       "<=")

/*********************************************************************
 * 
 * vector of pointer
 *
 *********************************************************************/
#define ASSERT_EQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(ptr, (__TEST__),				\
			    (const void**)(__LHS__),(__NLHS__),         \
			    (const void**)(__RHS__), (__NRHS__),        \
			    "==")


#define CHECK_EQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(ptr, (__TEST__),				\
                           (const void**)(__LHS__),(__NLHS__),          \
                           (const void**)(__RHS__), (__NRHS__),         \
                           "==")

#define ASSERT_NEQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(ptr, (__TEST__),				\
                            (const void**)(__LHS__),(__NLHS__),         \
                            (const void**)(__RHS__), (__NRHS__),        \
			    "!=")

#define CHECK_NEQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(ptr, (__TEST__),				\
                           (const void**)(__LHS__),(__NLHS__),          \
                           (const void**)(__RHS__), (__NRHS__),         \
                           "!=")

#define ASSERT_LT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(ptr, (__TEST__),				\
                            (const void**)(__LHS__),(__NLHS__),         \
                            (const void**)(__RHS__), (__NRHS__),        \
			    "<")

#define CHECK_LT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(ptr, (__TEST__),				\
                           (const void**)(__LHS__),(__NLHS__),          \
                           (const void**)(__RHS__), (__NRHS__),         \
                           "<")

#define ASSERT_LE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(ptr, (__TEST__),				\
                            (const void**)(__LHS__),(__NLHS__),         \
                            (const void**)(__RHS__), (__NRHS__),        \
			    "<=")

#define CHECK_LE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(ptr, (__TEST__),				\
                           (const void**)(__LHS__),(__NLHS__),          \
                           (const void**)(__RHS__), (__NRHS__),         \
                           "<=")

#define ASSERT_GT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(ptr, (__TEST__),				\
                            (const void**)(__LHS__),(__NLHS__),         \
                            (const void**)(__RHS__), (__NRHS__),        \
			    ">")

#define CHECK_GT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(ptr, (__TEST__),				\
                           (const void**)(__LHS__),(__NLHS__),          \
                           (const void**)(__RHS__), (__NRHS__),         \
                           ">")

#define ASSERT_GE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(ptr, (__TEST__),				\
                            (const void**)(__LHS__),(__NLHS__),         \
                            (const void**)(__RHS__), (__NRHS__),        \
			    ">=")

#define CHECK_GE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(ptr, (__TEST__),				\
                           (const void**)(__LHS__),(__NLHS__),          \
                           (const void**)(__RHS__), (__NRHS__),         \
                           ">=")

/*********************************************************************
 * 
 * c string
 *
 *********************************************************************/
#define ASSERT_EQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cstr, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cstr, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cstr, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cstr, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cstr, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of c-string
 *
 **********************************************************************/
#define ASSERT_EQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cstr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")

#define CHECK_EQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cstr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_CHECK_ARR_CMP__(cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cstr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cstr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">=")

assertion_t * unit_memchecker(unit_test_t              * tst,
			      struct memchecker_t      * memcheck,
			      const char               * file, 
			      int                        line);
                                         
void unit_run(FILE * fp,
              unit_context_t * ctx);

void unit_final_report(FILE * fp,
                       unit_context_t * ctx);

#define TEST(__SUITE__, __FUNC__)			\
  unit_create_test((__SUITE__), #__FUNC__, (__FUNC__))

/*****************************************************************
 * xmalloc assertions 
 *****************************************************************/
#define ASSERT_MEMCHECK(__TEST__)					\
  if(!unit_add_assertion( (__TEST__), memcheck_finalize()))		\
  {									\
    return;								\
  }

#define CHECK_MEMCHECK(__TEST__)				\
  unit_add_assertion( (__TEST__), memcheck_finalize())
				       
#endif

