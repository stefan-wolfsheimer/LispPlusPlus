/******************************************************************************
Copyright (c) 2021, Stefan Wolfsheimer

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
#include <lisp/util/argparse.h>
#include <stddef.h>

static void test_argparse_int(unit_test_t * tst)
{
  arg_parser_t parser;
  int int_val1;
  const char * argv1[] = {"-i", "1"};
  const char * argv2[] = {"--intval1",
                          "379839374982749249239824972397329473294294377778"};
  const char * argv3[] = {"--intval1",
                          "3798sdf"};

  memcheck_begin();
  ASSERT_FALSE(tst, init_arg_parser(&parser));
  arg_parser_add_int(&parser, &int_val1,
                     'i', "--intval1", 1,
                     "integer value 1");

  /* parse argv1 */
  int_val1 = 0;
  ASSERT_FALSE(tst, arg_parser_parse(&parser,
                                     sizeof(argv1)/sizeof(char*),
                                     argv1));
  ASSERT_EQ_I(tst, int_val1, 1);

  int_val1 = 0;
  ASSERT(tst, arg_parser_parse(&parser,
                               sizeof(argv2)/sizeof(char*),
                               argv2));

  int_val1 = 0;
  ASSERT(tst, arg_parser_parse(&parser,
                               sizeof(argv3)/sizeof(char*),
                               argv3));

  ASSERT_FALSE(tst, free_arg_parser(&parser));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_argparse_flags(unit_test_t * tst)
{
  arg_parser_t parser;
  int flag1;
  int flag2;
  int flag3;
  const char * argv1[] = {};
  const char * argv2[] = {"-f"};
  const char * argv3[] = {"-f", "--glag"};
  const char * argv4[] = {"-fx", "--glag"};

  memcheck_begin();
  ASSERT_FALSE(tst, init_arg_parser(&parser));
  flag1 = -1;
  flag2 = -1;
  flag3 = -1;
  arg_parser_add_flag(&parser, &flag1,
                      'f', "--flag",
                      "flag");
  arg_parser_add_flag(&parser, &flag2,
                      '\0', "--glag",
                      "flag 2");
  arg_parser_add_flag(&parser, &flag3,
                      'x', NULL,
                      "flag 3");

  /* parse argv1 */
  ASSERT_FALSE(tst, arg_parser_parse(&parser,
                                     sizeof(argv1)/sizeof(char*),
                                     argv1));
  ASSERT_EQ_I(tst, flag1, 0);
  ASSERT_EQ_I(tst, flag2, 0);
  ASSERT_EQ_I(tst, flag3, 0);

  flag1 = 0;
  flag2 = 0;
  flag3 = 0;
  ASSERT_FALSE(tst, arg_parser_parse(&parser,
                                     sizeof(argv2)/sizeof(char*),
                                     argv2));
  ASSERT_EQ_I(tst, flag1, 1);
  ASSERT_EQ_I(tst, flag2, 0);
  ASSERT_EQ_I(tst, flag3, 0);

  flag1 = 0;
  flag2 = 0;
  flag3 = 0;
  ASSERT_FALSE(tst, arg_parser_parse(&parser,
                                     sizeof(argv3)/sizeof(char*),
                                     argv3));
  ASSERT_EQ_I(tst, flag1, 1);
  ASSERT_EQ_I(tst, flag2, 0);
  ASSERT_EQ_I(tst, flag3, 0);


  flag1 = -1;
  flag2 = -1;
  flag3 = -1;
  ASSERT_FALSE(tst, arg_parser_parse(&parser,
                                     sizeof(argv4)/sizeof(char*),
                                     argv4));
  ASSERT_EQ_I(tst, flag1, 1);
  ASSERT_EQ_I(tst, flag2, 1);
  ASSERT_EQ_I(tst, flag3, 1);

  /*@todo test multiple flags! */
  ASSERT_FALSE(tst, free_arg_parser(&parser));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_argparse(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "argparse");
  TEST(suite, test_argparse_int);
  TEST(suite, test_argparse_flags);
}
