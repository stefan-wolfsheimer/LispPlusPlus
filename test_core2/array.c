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
#include <lisp/core/vm.h>
#include <lisp/core/cell.h>
#include <lisp/core/cell_iterator.h>
#include <lisp/core/array.h>
#include <lisp/core/error.h>

static void test_make_array(unit_test_t * tst)
{
  lisp_vm_t vm;
  lisp_cell_t cell;
  lisp_cell_iterator_t itr;
  memcheck_begin();
  ASSERT_EQ_I(tst, lisp_init_vm(&vm), LISP_OK);
  ASSERT_EQ_I(tst, lisp_make_array(&vm,
                                   &cell,
                                   5u), LISP_OK);
  ASSERT_EQ_I(tst, lisp_get_ref_count(&cell), 1u);
  ASSERT_FALSE(tst, lisp_is_atomic(&cell));
  ASSERT(tst, lisp_is_array(&cell));

  ASSERT_EQ_I(tst, lisp_first_child(&cell, &itr), LISP_OK);
  ASSERT(tst, lisp_cell_iterator_is_valid(&itr));

  ASSERT(tst, lisp_cell_next_child(&itr));
  ASSERT(tst, lisp_cell_next_child(&itr));
  ASSERT(tst, lisp_cell_next_child(&itr));
  ASSERT(tst, lisp_cell_next_child(&itr));
  ASSERT_FALSE(tst, lisp_cell_next_child(&itr));

  ASSERT(tst, lisp_is_root_cell(&cell));
  ASSERT_EQ_I(tst, lisp_free_vm(&vm), LISP_OK);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_array(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "array");
  TEST(suite, test_make_array);
}
