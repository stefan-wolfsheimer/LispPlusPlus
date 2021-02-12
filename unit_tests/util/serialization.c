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
#include <lisp/util/serialization.h>
#include <stddef.h>

#define TEST_STRUCT_1_N_FIELDS 2

/* header */
typedef struct test_struct_1_t
{
  int int_val1;
  int int_val2;
} test_struct_1_t;

typedef struct test_struct_1_context_t
{
  int default_int_value;
} test_struct_1_context_t;

extern serialization_field_t test_struct_1_fields_1[TEST_STRUCT_1_N_FIELDS];

/* implementation */
SERIAL_DEF_ACCESSOR(test_struct_1_t, int_val1);
SERIAL_DEF_ACCESSOR(test_struct_1_t, int_val2);

serialization_field_t test_struct_1_fields_1[TEST_STRUCT_1_N_FIELDS] =
{
  SERIAL_INT_FIELD(test_struct_1_t, int_val1, "value1", NULL,   NULL),
  SERIAL_INT_FIELD(test_struct_1_t, int_val2, NULL,     "05d%", NULL)
};

static void test_initializer_int_val2(serialization_field_t * f,
                                      void * obj,
                                      void * context)
{
  int * ptr = f->accessor(obj);
  *ptr = ((test_struct_1_context_t*)context)->default_int_value;
}

static void test_ser_constr(unit_test_t * tst)
{
  memcheck_begin();
  test_struct_1_context_t context;
  test_struct_1_t str1;
  test_struct_1_t str2;

  context.default_int_value = 42;
  serialization_set_initializer(test_struct_1_fields_1,
                                TEST_STRUCT_1_N_FIELDS,
                                "int_val2",
                                test_initializer_int_val2);
  serialization_init(test_struct_1_fields_1,
                     TEST_STRUCT_1_N_FIELDS,
                     &str1,
                     &context);
  serialization_init(test_struct_1_fields_1,
                     TEST_STRUCT_1_N_FIELDS,
                     &str2,
                     &context);
  ASSERT_EQ_I(tst, str1.int_val1, 0);
  ASSERT_EQ_I(tst, str1.int_val2, 42);
  ASSERT(tst,
         serialization_eq(test_struct_1_fields_1,
                          TEST_STRUCT_1_N_FIELDS,
                          &str1,
                          &str2));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_serialization(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "serial");
  TEST(suite, test_ser_constr);
}
