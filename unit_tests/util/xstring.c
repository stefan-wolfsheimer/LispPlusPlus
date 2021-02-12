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
#include <lisp/util/xstring.h>
#include <string.h>

static void test_strcpy_null(unit_test_t * tst)
{
  memcheck_begin();
  memcheck_expected_alloc(0);
  ASSERT_EQ_PTR(tst, alloc_strcpy(NULL), NULL);
  ASSERT_EQ_U(tst, memcheck_retire_mocks(),1)
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_strcpy(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc";
  char * cpy = alloc_strcpy(from);
  ASSERT_NEQ_PTR(tst, cpy, from);
  ASSERT_EQ_CSTR(tst, cpy, from);
  FREE(cpy);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_strcpy_fail(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc";
  memcheck_expected_alloc(0);
  char * cpy = alloc_strcpy(from);
  ASSERT_EQ_PTR(tst, cpy, NULL);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_sprintf_empty(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc";
  char * cpy = alloc_sprintf(from);
  ASSERT_NEQ_PTR(tst, cpy, NULL);
  ASSERT_NEQ_PTR(tst, cpy, from);
  ASSERT_EQ_CSTR(tst, cpy, from);
  FREE(cpy);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_sprintf(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc %d %s";
  char * cpy = alloc_sprintf(from, 1, from);
  ASSERT_NEQ_PTR(tst, cpy, NULL);
  ASSERT_NEQ_PTR(tst, cpy, from);
  ASSERT_EQ_CSTR(tst, cpy, "abc 1 abc %d %s");
  FREE(cpy);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_sprintf_fail(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc %d %s";
  memcheck_expected_alloc(0);
  char * cpy = alloc_sprintf(from, 1, from);
  ASSERT_EQ_PTR(tst, cpy, NULL);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_xstring(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "xstring");
  TEST(suite, test_strcpy_null);
  TEST(suite, test_strcpy);
  TEST(suite, test_strcpy_fail);
  TEST(suite, test_sprintf_empty);
  TEST(suite, test_sprintf);
  TEST(suite, test_sprintf_fail);
}
