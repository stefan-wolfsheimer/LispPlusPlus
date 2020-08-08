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
#include <lpp/core/types/type_traits.h>
#include <catch.hpp>
#include <type_traits>

struct A {};
struct B {};
struct C {};
struct D {};

template<bool BV1, bool BV2, bool BV3>
struct ABCD : public Lisp::Conditional<std::integral_constant<bool, BV1>,
                                       A,
                                       std::integral_constant<bool, BV2>,
                                       B,
                                       std::integral_constant<bool, BV3>,
                                       C,
                                       D>
{
};

TEST_CASE("Conditional", "[Traits]")
{
  REQUIRE(std::is_same<Lisp::Conditional<std::true_type, A, B>::type, A>::value);
  REQUIRE(std::is_same<Lisp::Conditional<std::false_type, A, B>::type, B>::value);
  
  REQUIRE(std::is_same<ABCD<false, false, false>::type, D>::value);
  REQUIRE(std::is_same<ABCD<false, false, true>::type, C>::value);
  REQUIRE(std::is_same<ABCD<false, true, false>::type, B>::value);
  REQUIRE(std::is_same<ABCD<false, true, true>::type, B>::value);
  REQUIRE(std::is_same<ABCD<true, false, false>::type, A>::value);
  REQUIRE(std::is_same<ABCD<true, false, true>::type, A>::value);
  REQUIRE(std::is_same<ABCD<true, true, false>::type, A>::value);
  REQUIRE(std::is_same<ABCD<true, true, true>::type, A>::value);
}
