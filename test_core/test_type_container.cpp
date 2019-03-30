/******************************************************************************
Copyright (c) 2019, Stefan Wolfsheimer

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
#include <lpp/core/memory/type_container.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/basic_type.h>
#include <lpp/core/object.h>
#include <lpp/core/types/cons.h>
#include <catch.hpp>

using TypeContainer = Lisp::TypeContainer;
using IntegerType = Lisp::IntegerType;
using Cons = Lisp::Cons;
using Object = Lisp::Object;
using BasicType = Lisp::BasicType;

TEST_CASE("type_life_cycle", "[TypeContainer]")
{
  /* @todo reactivate this test */
  /*TypeContainer container;
  Object intType1(container.make<IntegerType>());
  Object intType2(container.make<IntegerType>());
  REQUIRE(intType1.isA<BasicType>());
  REQUIRE(intType2.isA<BasicType>());
  REQUIRE(intType1.as<BasicType>() == intType2.as<BasicType>());
  Object consType1(container.make<Cons>());
  REQUIRE(consType1.isA<BasicType>());
  REQUIRE(intType1.as<BasicType>() != consType1.as<BasicType>()); */
}
