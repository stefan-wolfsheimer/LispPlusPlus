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
#include "assertion.h"
#include "xstring.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define COLOR_NORMAL   "\x1B[0m"
#define COLOR_RED      "\x1B[31m"
#define COLOR_GREEN    "\x1B[32m"
#define COLOR_YELLOW   "\x1B[33m"
#define COLOR_BLUE     "\x1B[34m"
#define COLOR_MAGENTA  "\x1B[35m"
#define COLOR_CYAN     "\x1B[36m"
#define COLOR_WHITE    "\x1B[37m"

#ifdef WITH_COLOR 
#define ASSERTION_COLOR 1
#else 
#define ASSERTION_COLOR 0
#endif 

assertion_handler_t _assertion_handler = 
{
 handler_cb : NULL,
 user_data  : NULL,
 fp         : NULL,
 use_stderr : 1
};

static void _print_result(FILE * fp, int success, int color) 
{
  if(color)
  {
    if(success) fprintf(fp, COLOR_GREEN"[PASSED]"COLOR_NORMAL);
    else fprintf(fp, COLOR_RED"[FAILED]"COLOR_NORMAL);
  }
  else 
  {
    if(success) fprintf(fp, "[PASSED]");
    else fprintf(fp, "[FAILED]");
  }
}

assertion_t * assertion_create(const char * file, 
			       int          line)
{
  int old = memcheck_enable(0);
  assertion_t * assertion = malloc(sizeof(assertion_t));
  if(assertion == NULL) { return assertion; }
  assertion->expect         = NULL;
  assertion->expect_explain = NULL;
  assertion->file           = alloc_strcpy(file);
  assertion->line           = line;
  assertion->success        = 0;
  assertion->is_exception   = 0;
  assertion->next           = NULL;
  memcheck_enable(old);
  return assertion;
}

assertion_t * assertion_create_message(const char * file,
				       int          line,
				       const char * expect,
				       const char * explain,
				       int          is_exception,
				       int          success)
{
  int old = memcheck_enable(0);
  assertion_t * assertion = malloc(sizeof(assertion_t));
  if(assertion == NULL) { return assertion; }
  assertion->expect         = alloc_strcpy(expect);
  assertion->expect_explain = alloc_strcpy(explain);
  assertion->file           = alloc_strcpy(file);
  assertion->line           = line;
  assertion->success        = success;
  assertion->is_exception   = is_exception;
  assertion->next           = NULL;
  memcheck_enable(old);
  return assertion;
}


assertion_t * assertion_free(assertion_t * assertion)
{
  assertion_t * ret = assertion->next;
  if(assertion->expect) 
  {
    free(assertion->expect);
  }
  if(assertion->expect_explain) 
  {
    free(assertion->expect_explain);
  }
  if(assertion->file)
  {
    free(assertion->file);
  }
  free(assertion);
  return ret;
}

assertion_t * assertion_invert(assertion_t * assertion)
{
  if(assertion) 
  {
    assertion->success = assertion->success ? 0 : 1;
    if(assertion->expect) 
    {
      char * tmp = assertion->expect;
      int old = memcheck_enable(0);
      if(assertion->is_exception) 
      {
	assertion->expect = alloc_sprintf("expected %s", tmp);
      }
      else 
      {
	assertion->expect = alloc_sprintf("not (%s)", tmp);
      }
      free(tmp);
      memcheck_enable(old);
    }
    else 
    {
      int old = memcheck_enable(0);
      assertion->expect = alloc_strcpy("not (...)");
      memcheck_enable(old);
    }
  }
  return assertion;
}

void assertion_print(FILE        * fp,
		     assertion_t * assertion,
		     int           color)
{
  _print_result(fp, assertion->success, color);
  if(assertion->is_exception) 
  {
    fprintf(fp, "    %s:%d: exception <%s>\n",
	    assertion->file,
	    assertion->line,
	    assertion->expect);
  }
  else 
  {
    fprintf(fp, "    %s:%d: assertion <%s>\n",
	    assertion->file, 
	    assertion->line, 
	    assertion->expect);
  }
  if(assertion->expect_explain) 
  {
    const char * begin = assertion->expect_explain;
    int size = snprintf(NULL, 0, "            %s:%d: ",
			assertion->file,
			assertion->line);
    while(*begin) 
    {
      const char * ptr = begin;
      while(*ptr && *ptr!= '\n')
      {
	ptr++;
      }
      char * buff = malloc(size + (ptr-begin) + 2);
      int i;
      for(i = 0; i < size; i++) 
      {
	buff[i] = ' ';
      }
      ptr = begin;
      while(*ptr && *ptr!= '\n')
      {
	buff[i++] = *ptr;
	ptr++;
      }
      buff[i] = '\0';
      fprintf(fp, "%s\n", buff);
      begin = ptr;
      if(*begin == '\n')
      { 
	begin++;
      }
      free(buff);
    }
  }
}

/*********************************************************************
 * 
 * raise exception 
 *
 **********************************************************************/
void assertion_raise(assertion_t * assertion)
{
  if(assertion != NULL && !assertion->success) 
  {
    int doraise = 1;
    if(_assertion_handler.handler_cb) 
    {
      doraise = _assertion_handler.handler_cb(assertion, 
					      _assertion_handler.user_data);
    }
    FILE * fp = (_assertion_handler.fp == NULL && _assertion_handler.use_stderr) ?
      stderr : 
      _assertion_handler.fp;
    if(fp != NULL) 
    {
      assertion_print(fp, assertion, ASSERTION_COLOR);
    }
    assertion_free(assertion);
    if(doraise) 
    {
      raise(SIGABRT);
    }
  }
  else if(assertion != NULL) 
  {
    assertion_free(assertion);
  }
}

const assertion_handler_t 
assertion_register_handler(const assertion_handler_t handler)
{
  assertion_handler_t ret       = _assertion_handler;
  _assertion_handler.handler_cb = handler.handler_cb;
  _assertion_handler.user_data  = handler.user_data;
  _assertion_handler.fp         = handler.fp;
  _assertion_handler.use_stderr = handler.use_stderr;
  return ret;
}


#define __CREATE_CMP__(__CMP_TYPE__, __TYPE__)				\
  int assertion_cmp_##__CMP_TYPE__(__TYPE__ lhs, __TYPE__ rhs)		\
  {									\
    if(lhs == rhs)							\
    {									\
      return 0;								\
    }									\
    else if(lhs < rhs)							\
    {									\
      return -1;							\
    }									\
    else								\
    {									\
      return 1;								\
    }									\
  }

#define __CREATE_CMP_ARR__(__CMP_TYPE__, __TYPE__)			\
  int assertion_cmp_arr_##__CMP_TYPE__(const __TYPE__ * lhs,		\
				       size_t nlhs,			\
				       const __TYPE__ * rhs,		\
				       size_t nrhs)			\
  {									\
    size_t i;								\
    for(i = 0; i < nlhs; i++)						\
    {									\
      if(i >= nrhs)							\
      {									\
	return 1;							\
      }									\
      if(lhs[i] < rhs[i])						\
      {									\
	return -1;							\
      }									\
      else if(lhs[i] > rhs[i])						\
      {									\
	return 1;							\
      }									\
    }									\
    if(i < nrhs)							\
    {									\
      return -1;							\
    }									\
    else								\
    {									\
      return 0;								\
    }									\
  }


#define __CREATE_ASSERTION_CMP__(__CMP_TYPE__, __TYPE__, __FMT__)	\
  assertion_t *								\
  assertion_create_cmp_##__CMP_TYPE__(const char * file,		\
				      int          line,		\
				      const char * lhs_expr,		\
				      const char * rhs_expr,		\
				      __TYPE__     lhs,			\
				      __TYPE__     rhs,			\
				      const char * op,			\
				      int          ofailure)		\
  {									\
  int success =								\
      ( !strcmp(op,"==") && !assertion_cmp_##__CMP_TYPE__(lhs, rhs))    || \
      ( !strcmp(op,"!=") &&  assertion_cmp_##__CMP_TYPE__(lhs, rhs))    || \
      ( !strcmp(op,"<")  &&  assertion_cmp_##__CMP_TYPE__(lhs, rhs)<0)  || \
      ( !strcmp(op, ">") &&  assertion_cmp_##__CMP_TYPE__(lhs, rhs)>0)  || \
      ( !strcmp(op,"<=") &&  assertion_cmp_##__CMP_TYPE__(lhs, rhs)<=0) || \
      ( !strcmp(op, ">=")&&  assertion_cmp_##__CMP_TYPE__(lhs, rhs)>=0);   \
    if(!success || !ofailure)						\
    {									\
      assertion_t * assertion = assertion_create(file, line);		\
      if(!assertion)							\
      {									\
	return NULL;							\
      }									\
      assertion->success = success;					\
      int old = memcheck_enable(0);					\
      assertion->expect = alloc_sprintf("%s%s%s",			\
					lhs_expr, op, rhs_expr);	\
      char * lhs_str = alloc_sprintf(#__FMT__, lhs);			\
      char * rhs_str = alloc_sprintf(#__FMT__, rhs);			\
      int cmp_lhs = strcmp(lhs_str, lhs_expr);				\
      int cmp_rhs = strcmp(rhs_str, rhs_expr);				\
      if(cmp_lhs && cmp_rhs)						\
      {									\
	assertion->expect_explain = alloc_sprintf("with %s==%s\n"	\
						  "     %s==%s",	\
						  lhs_expr,		\
						  lhs_str,		\
						  rhs_expr,		\
						  rhs_str);		\
      }                                           			\
      else if(cmp_lhs || cmp_rhs)                                       \
      {									\
        assertion->expect_explain = alloc_sprintf("with %s==%s",	\
						  cmp_lhs ?		\
						  lhs_expr : rhs_expr,	\
						  cmp_lhs ?		\
						  lhs_str : rhs_str);	\
      }                                          			\
      free(rhs_str);	                                                \
      free(lhs_str);	                                                \
      memcheck_enable(old);						\
      return assertion;							\
    }									\
    else								\
    {									\
      return NULL;							\
    }									\
}

/* @todo printers for arrays */
#define __CREATE_ASSERTION_CMP_ARR__(__CMP_TYPE__, __TYPE__, __FMT__)	\
  assertion_t *								\
  assertion_create_cmp_arr_##__CMP_TYPE__(const char *     file,	\
					  int              line,	\
					  const char * lhs_expr,	\
					  const char * rhs_expr,	\
					  const __TYPE__ * lhs,		\
					  size_t           nlhs,	\
					  const __TYPE__ * rhs,		\
					  size_t nrhs,			\
					  const char * op,		\
					  int          ofailure)	\
  {									\
    int success =							\
      ( !strcmp(op,"==") && !assertion_cmp_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs))    || \
      ( !strcmp(op,"!=") &&  assertion_cmp_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs))    || \
      ( !strcmp(op,"<")  &&  assertion_cmp_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs)<0)  || \
      ( !strcmp(op, ">") &&  assertion_cmp_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs)>0)  || \
      ( !strcmp(op,"<=") &&  assertion_cmp_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs)<=0) || \
      ( !strcmp(op, ">=")&&  assertion_cmp_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs)>=0); \
    if(!success || !ofailure)						\
    {									\
      assertion_t * assertion = assertion_create(file, line);		\
      if(!assertion)							\
      {									\
	return NULL;							\
      }									\
      assertion->success = success;					\
      return assertion;							\
    }									\
    else								\
    {									\
      return NULL;							\
    }									\
  }


/*********************************************************************
 * 
 * assertions for boolean
 *
 *********************************************************************/

assertion_t * assertion_create_true(const char * file, 
				    int          line,
				    const char * expr_str,
				    int          expr,
				    int          only_on_failure)
{
  if(!expr || !only_on_failure) 
  {
    assertion_t * assertion = assertion_create(file, line);
    if(!assertion) return NULL;
    assertion->success = expr;
    int old = memcheck_enable(0);
    assertion->expect = alloc_sprintf("%s is true", expr_str);
    memcheck_enable(old);
    return assertion;
  }
  else
  {
    return NULL;
  }
}

assertion_t * assertion_create_false(const char * file, 
				     int          line,
				     const char * expr_str,
				     int          expr,
				     int          only_on_failure)
{
  if(expr || !only_on_failure) 
  {
    assertion_t * assertion = assertion_create(file, line);
    if(!assertion) return NULL;
    assertion->success = !expr;
    int old = memcheck_enable(0);
    assertion->expect = alloc_sprintf("%s is true", expr_str);
    memcheck_enable(old);
    return assertion;
  }
  else
  {
    return NULL;
  }
}


/*********************************************************************
 * 
 * assertions for int
 *
 *********************************************************************/
__CREATE_CMP__(i, int);
__CREATE_ASSERTION_CMP__(i, int, %d);

/*********************************************************************
 * 
 * assertions for vector of int
 *
 *********************************************************************/
__CREATE_CMP_ARR__(i, int);
__CREATE_ASSERTION_CMP_ARR__(i, int, %d);

/*********************************************************************
 * 
 * assertions for unsigned
 *
 *********************************************************************/
__CREATE_CMP__(u, unsigned);
__CREATE_ASSERTION_CMP__(u, unsigned int, %u);

/*********************************************************************
 * 
 * assertions for vector of unsigned
 *
 *********************************************************************/
__CREATE_CMP_ARR__(u, unsigned);
__CREATE_ASSERTION_CMP_ARR__(u, unsigned, %u);

/*********************************************************************
 * 
 * assertions for pointer
 *
 *********************************************************************/
__CREATE_CMP__(ptr, const void *);
__CREATE_ASSERTION_CMP__(ptr, const void *, %p);

/*********************************************************************
 * 
 * assertions for vector of pointer
 *
 *********************************************************************/
__CREATE_CMP_ARR__(ptr, const void *);
__CREATE_ASSERTION_CMP_ARR__(ptr, const void *, %p);

/*********************************************************************
 * 
 * assertions for c-string
 *
 *********************************************************************/
int assertion_cmp_cstr(const char * lhs, const char * rhs)
{
  return strcmp(lhs, rhs);
}
__CREATE_ASSERTION_CMP__(cstr, const char *, "%s");


/*********************************************************************
 * 
 * assertions for array of c-string
 *
 *********************************************************************/
int assertion_cmp_arr_cstr(const char ** lhs, size_t nlhs,
			   const char ** rhs, size_t nrhs)
{
  size_t i;
  for(i = 0; i < nlhs; i++)
  {
    if(i >= nrhs)
    {
      return 1;
    }
    if(strcmp(lhs[i], rhs[i]) < 0)
    {
      return -1;
    }
    else if(strcmp(lhs[i],rhs[i]) > 0)
    {
      return 1;
    }
  }
  if(i < nrhs)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}
__CREATE_ASSERTION_CMP_ARR__(cstr, const char *, "%s");
