/******************************************************************************
Copyright (c) 2018, Stefan Wolfsheimer

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
#include "test_random_access_iterator.h"

#include <catch.hpp>
#include <lpp/core/memory/cons_pages.h>
#include <lpp/core/types/cons.h>

using ConsPages = Lisp::ConsPages;
using BasicCons = Lisp::BasicCons;

TEST_CASE("cons_pages_life_time", "[ConsPages]")
{
  ConsPages pages(4);
  REQUIRE(testRandomAccessIterator({}, pages.cbegin(), pages.cend()));
  REQUIRE(pages.getPageSize() == 4u);
  REQUIRE(pages.getNumAllocated() == 0u);
  REQUIRE(pages.getNumVoid() == 0u);
  BasicCons * cons1 = pages.next();
  REQUIRE(testRandomAccessIterator({cons1},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumAllocated() == 4u);
  REQUIRE(pages.getNumVoid() == 3u);
  BasicCons * cons2 = pages.next();
  REQUIRE(testRandomAccessIterator({cons1, cons2},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumAllocated() == 4u);
  REQUIRE(pages.getNumVoid() == 2u);
  BasicCons * cons3 = pages.next();
  REQUIRE(testRandomAccessIterator({cons1, cons2, cons3},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumAllocated() == 4u);
  REQUIRE(pages.getNumVoid() == 1u);
  BasicCons * cons4 = pages.next();
  REQUIRE(testRandomAccessIterator({cons1, cons2, cons3, cons4},
                                   pages.cbegin(), pages.cend()));

  REQUIRE(pages.getNumAllocated() == 4u);
  REQUIRE(pages.getNumVoid() == 0u);
  BasicCons * cons5 = pages.next();
  REQUIRE(testRandomAccessIterator({cons1, cons2, cons3, cons4, cons5},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumAllocated() == 8u);
  REQUIRE(pages.getNumVoid() == 3u);
  pages.recycle(cons1);
  REQUIRE(testRandomAccessIterator({cons1, cons2, cons3, cons4, cons5},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumRecycled() == 1u);
  REQUIRE(pages.getNumVoid() == 4u);
  REQUIRE(pages.next() == cons1);
  REQUIRE(testRandomAccessIterator({cons1, cons2, cons3, cons4, cons5},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumRecycled() == 0u);
  REQUIRE(pages.getNumAllocated() == 8u);
  pages.recycle(cons2);
  pages.recycle(cons3);
  pages.recycle(cons4);
  pages.recycle(cons5);
  REQUIRE(testRandomAccessIterator({cons1, cons2, cons3, cons4, cons5},
                                   pages.cbegin(), pages.cend()));
  REQUIRE(pages.getNumVoid() == 7u);
  REQUIRE(pages.getNumRecycled() == 4u);
  REQUIRE(pages.getNumAllocated() == 8u);
  REQUIRE(pages.next() == cons5);
  REQUIRE(pages.next() == cons4);
  REQUIRE(pages.next() == cons3);
  REQUIRE(pages.next() == cons2);
  REQUIRE(pages.getNumRecycled() == 0u);
  REQUIRE(pages.getNumAllocated() == 8u);
  REQUIRE(pages.getNumVoid() == 3u);
}

