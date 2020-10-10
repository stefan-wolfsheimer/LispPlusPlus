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
#ifndef __ASSERTION_H__
#define __ASSERTION_H__
#include <stdlib.h>
#include <stdio.h>

typedef struct assertion_t
{
  char               * expect;
  char               * expect_explain;
  char               * file;
  int                  line;
  int                  success;
  int                  is_exception;
  struct assertion_t * next;
} assertion_t;

typedef struct assertion_handler_t 
{
  int (*handler_cb)(assertion_t * assertion,
		    void        * user_data);
  void * user_data;
  FILE * fp;
  int    use_stderr; /* use stderr if fp == NULL */
} assertion_handler_t;

assertion_t * assertion_create(const char * file, 
			       int          line);

assertion_t * assertion_create_message(const char * file,
				       int          line,
				       const char * expect,
				       const char * explain,
				       int          is_exception,
				       int          success);

/** 
 * Free the assertion if it has only one reference.
 * @return next assertion in the list
 */
assertion_t * assertion_free(assertion_t * assertion);

/** 
 * Invert the assertion. 
 */
assertion_t * assertion_invert(assertion_t * assertion);

void assertion_print(FILE * fp, assertion_t * ass, int color);

/**
 * Raise an exception when assertion is not successful
 */
void assertion_raise(assertion_t * assertion);

const assertion_handler_t 
assertion_register_handler(const assertion_handler_t handler);

/*********************************************************************
 * 
 * assertions boolean
 *
 *********************************************************************/
assertion_t * assertion_create_true(const char * file, 
				    int          line,
				    const char * expr_str,
				    int          expr,
				    int          only_on_failure);

assertion_t * assertion_create_false(const char * file, 
				     int          line,
				     const char * expr_str,
				     int          expr,
				     int          only_on_failure);

/*********************************************************************
 * 
 * assertions for int 
 *
 *********************************************************************/
int           assertion_cmp_i(int lhs, 
			      int rhs);
assertion_t * assertion_create_cmp_i(const char * file, 
				     int          line,
				     const char * lhs_expr,
				     const char * rhs_expr,
				     int          lhs,
				     int          rhs,
				     const char * cmp_op,
				     int          only_on_failure);

/*********************************************************************
 * 
 * assertions for vector of int 
 *
 *********************************************************************/
int assertion_cmp_arr_i(const int * lhs, size_t nlhs,
			const int * rhs, size_t nrhs);
assertion_t * assertion_create_cmp_arr_i(const char * file, 
					 int          line,
					 const char * lhs_expr,
					 const char * rhs_expr,
					 const int *  lhs,
					 size_t       nlhs,
					 const int *  rhs,
					 size_t       nrhs,
					 const char * cmp_op,
					 int          only_on_failure);


/*********************************************************************
 * 
 * assertions for unsigned int 
 *
 *********************************************************************/
int assertion_cmp_u(unsigned lhs,
		    unsigned rhs);
assertion_t * assertion_create_cmp_u(const char * file, 
				     int          line,
				     const char * lhs_expr,
				     const char * rhs_expr,
				     unsigned int lhs,
				     unsigned int rhs,
				     const char * cmp_op,
				     int          only_on_failure);

/*********************************************************************
 * 
 * assertions for vector of unsigned int 
 *
 *********************************************************************/
int assertion_cmp_arr_u(const unsigned * lhs, size_t nlhs,
			const unsigned * rhs, size_t nrhs);

assertion_t * assertion_create_cmp_arr_u(const char *      file, 
					 int               line,
					 const char *      lhs_expr,
					 const char *      rhs_expr,
					 const unsigned *  lhs,
					 size_t            nlhs,
					 const unsigned *  rhs,
					 size_t            nrhs,
					 const char *      cmp_op,
					 int               only_on_failure);

/*********************************************************************
 * 
 * assertions for pointer
 *
 *********************************************************************/
int assertion_cmp_ptr(const void * lhs, 
		      const void * rhs);
assertion_t * assertion_create_cmp_ptr(const char * file, 
				       int          line,
				       const char * lhs_expr,
				       const char * rhs_expr,
				       const void * lhs,
				       const void * rhs,
				       const char * cmp_op,
				       int          only_on_failure);

/*********************************************************************
 * 
 * assertions for vector of pointer
 *
 *********************************************************************/
int assertion_cmp_arr_ptr(const void ** lhs, size_t nlhs,
			  const void ** rhs, size_t nrhs);

assertion_t * assertion_create_cmp_arr_ptr(const char *      file, 
					   int               line,
					   const char *      lhs_expr,
					   const char *      rhs_expr,
					   const void **  lhs,
					   size_t            nlhs,
					   const void **     rhs,
					   size_t            nrhs,
					   const char *      cmp_op,
					   int               only_on_failure);

/*********************************************************************
 * 
 * assertions for c-string
 *
 *********************************************************************/
int assertion_cmp_cstr(const char * lhs,
		       const char * rhs);
assertion_t * assertion_create_cmp_cstr(const char * file, 
					int          line,
					const char * lhs_expr,
					const char * rhs_expr,
					const char * lhs,
					const char * rhs,
					const char * cmp_op,
					int          only_on_failure);

/*********************************************************************
 * 
 * assertions for array of c-string
 *
 *********************************************************************/
int assertion_cmp_arr_cstr(const char ** lhs, size_t nlhs,
			   const char ** rhs, size_t nrhs);


assertion_t * assertion_create_cmp_arr_cstr(const char *      file, 
					    int               line,
					    const char *      lhs_expr,
					    const char *      rhs_expr,
					    const char **  lhs,
					    size_t            nlhs,
					    const char **     rhs,
					    size_t            nrhs,
					    const char *      cmp_op,
					    int               only_on_failure);



/*********************************************************************
 * 
 * macros to create check and assertion function declarations and
 * assertion macros
 *
 *********************************************************************/
#ifdef DEBUG
#define __CREATE_REQUIRE_CMP__(__TYPE__, __LHS__, __RHS__,_OP_)		\
  assertion_raise(assertion_create_cmp_##__TYPE__(__FILE__,		\
						  __LINE__,		\
						  (#__LHS__),		\
						  (#__RHS__),		\
						  (__LHS__),		\
						  (__RHS__),		\
						  (_OP_),		\
						  1))

#define __CREATE_REQUIRE_ARR_CMP__(__TYPE__,				\
				  __LHS__,__NLHS__,			\
				  __RHS__,__NRHS__,_OP_)		\
  assertion_raise(assertion_create_cmp_arr_##__TYPE__(__FILE__,		\
						      __LINE__,		\
						      (#__LHS__),	\
						      (#__RHS__),	\
						      (__LHS__),	\
						      (__NLHS__),	\
						      (__RHS__),	\
						      (__NRHS__),	\
						      _OP_,		\
						      1))
#else 
#define __CREATE_REQUIRE_CMP__(__TYPE__, __LHS__, __RHS__,_OP_)
#define __CREATE_REQUIRE_ARR_CMP__(__TYPE__, LHS,NLHS,RHS,NRHS,_OP_)
#endif

/*********************************************************************
 * 
 * boolean
 *
 **********************************************************************/
#ifdef DEBUG
#define REQUIRE(__EXPR__)						\
  assertion_raise(assertion_create_true(__FILE__, __LINE__,		\
					#__EXPR__, (__EXPR__), 1))	\

#define REQUIRE_FALSE(__EXPR__)						\
  assertion_raise(assertion_create_false(__FILE__, __LINE__,		\
					#__EXPR__, (__EXPR__), 1))	\

#else
#define REQUIRE(__EXPR__)
#define REQUIRE_FALSE(__EXPR__)
#endif


/*********************************************************************
 * 
 * int
 *
 **********************************************************************/
#define REQUIRE_EQ_I( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(i, __LHS__, __RHS__, "==")

#define REQUIRE_NEQ_I( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(i, __LHS__, __RHS__, "!=")

#define REQUIRE_GT_I( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(i, __LHS__, __RHS__, ">")

#define REQUIRE_GE_I( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(i, __LHS__, __RHS__, ">=")

#define REQUIRE_LT_I( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(i, __LHS__, __RHS__, "<")

#define REQUIRE_LE_I( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(i, __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of int
 *
 **********************************************************************/
#define REQUIRE_EQ_ARR_I( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(i,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")

#define REQUIRE_NEQ_ARR_I( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(i,				\
			    (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			    "!=")

#define REQUIRE_LT_ARR_I( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(i,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")
#define REQUIRE_LE_ARR_I( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(i,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")

#define REQUIRE_GT_ARR_I( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(i,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define REQUIRE_GE_ARR_I( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(i,				\
			    (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			     ">=")
  

/*********************************************************************
 * 
 * unsigned int
 *
 **********************************************************************/
#define REQUIRE_EQ_U( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(u, __LHS__, __RHS__, "==")

#define REQUIRE_NEQ_U( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(u, __LHS__, __RHS__, "!=")

#define REQUIRE_GT_U( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(u, __LHS__, __RHS__, ">")

#define REQUIRE_GE_U( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(u, __LHS__, __RHS__, ">=")

#define REQUIRE_LT_U( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(u, __LHS__, __RHS__, "<")

#define REQUIRE_LE_U( __LHS__, __RHS__)				\
  __CREATE_REQUIRE_CMP__(u, __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector unsigned int
 *
 **********************************************************************/
#define REQUIRE_EQ_ARR_U( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(u,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")

#define REQUIRE_NEQ_ARR_U( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(u,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")

#define REQUIRE_LT_ARR_U( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(u,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")

#define REQUIRE_LE_ARR_U( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(u,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")

#define REQUIRE_GT_ARR_U( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(u,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define REQUIRE_GE_ARR_U( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(u,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")

/*********************************************************************
 * 
 * pointer
 *
 *********************************************************************/
#define REQUIRE_EQ_PTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(ptr, __LHS__, __RHS__, "==")

#define REQUIRE_NEQ_PTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(ptr, __LHS__, __RHS__, "!=")

#define REQUIRE_GT_PTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(ptr, __LHS__, __RHS__, ">")

#define REQUIRE_GE_PTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(ptr, __LHS__, __RHS__, ">=")

#define REQUIRE_LT_PTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(ptr, __LHS__, __RHS__, "<")

#define REQUIRE_LE_PTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(ptr, __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of pointer
 *
 *********************************************************************/
#define REQUIRE_EQ_ARR_PTR( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(ptr,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")

#define REQUIRE_NEQ_ARR_PTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(ptr,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")

#define REQUIRE_LT_ARR_PTR( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(ptr,				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")

#define REQUIRE_LE_ARR_PTR( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(ptr,					\
			     (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			     "<=")

#define REQUIRE_GT_ARR_PTR( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(ptr,					\
			     (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			     ">")

#define REQUIRE_GE_ARR_PTR( __LHS__,__NLHS__, __RHS__,__NRHS__)		\
  __CREATE_REQUIRE_ARR_CMP__(ptr,					\
			     (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			     ">=")

/*********************************************************************
 * 
 * c string
 *
 *********************************************************************/
#define REQUIRE_EQ_CSTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(cstr, __LHS__, __RHS__, "==")

#define REQUIRE_NEQ_CSTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(cstr, __LHS__, __RHS__, "!=")

#define REQUIRE_GT_CSTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(cstr, __LHS__, __RHS__, ">")

#define REQUIRE_GE_CSTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(cstr, __LHS__, __RHS__, ">=")

#define REQUIRE_LT_CSTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(cstr, __LHS__, __RHS__, "<")

#define REQUIRE_LE_CSTR( __LHS__, __RHS__)			\
  __CREATE_REQUIRE_CMP__(cstr, __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of c-string
 *
 **********************************************************************/
#define REQUIRE_EQ_ARR_CSTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(cstr,					\
			     (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			    "==")

#define REQUIRE_NEQ_ARR_CSTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(cstr,					\
			     (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			     "!=")

#define REQUIRE_LT_ARR_CSTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(cstr,					\
			     (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			     "<")

#define REQUIRE_LE_ARR_CSTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(cstr,					\
			     (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			    "<=")

#define REQUIRE_GT_ARR_CSTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(cstr,					\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			     ">")

#define REQUIRE_GE_ARR_CSTR( __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_REQUIRE_ARR_CMP__(cstr,					\
			    (__LHS__),(__NLHS__),			\
			     (__RHS__), (__NRHS__),			\
			     ">=")


#endif
