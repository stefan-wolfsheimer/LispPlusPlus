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
#include <catch.hpp>
#include <lpp/core/vm.h>
#include <lpp/core/types/array.h>
#include <lpp/core/cell.h>

using Cell = Lisp::Cell;
using Array = Lisp::Array;
using Collectible = Lisp::Collectible;
using Container = Lisp::Collectible;
using Object = Lisp::Object;

TEST_CASE("nil_is_not_a_array", "[Array]")
{
  REQUIRE_FALSE(Lisp::nil.isA<Array>());
  REQUIRE_FALSE(Lisp::nil.isA<Container>());
}

TEST_CASE("nil_as_array_is_pullptr", "[Array]")
{
  REQUIRE(Lisp::nil.as<Array>() == nullptr);
}

TEST_CASE("array_is_a_array", "[Array]")
{
  Lisp::Vm vm;
  auto obj = vm.array();
  REQUIRE(obj.isA<Array>());
  REQUIRE(obj.isA<Container>());
  REQUIRE(obj.isA<Collectible>());
  REQUIRE(obj.as<Array>());
}

TEST_CASE("array_has_refcount_1", "[Array]")
{
  Lisp::Vm vm;
  auto obj = vm.array();
  REQUIRE(obj.as<Array>()->getRefCount() == 1u);
  REQUIRE(obj.as<Array>()->isRoot());
}

TEST_CASE("array_hash", "[Array]")
{
  Lisp::Vm vm;
  std::hash<Cell> h;
  std::equal_to<Cell> eq;
  auto o1 = vm.array();
  auto o2 = vm.array();
  REQUIRE(h(o1) == h(o1));
  REQUIRE(h(o1) != h(o2));
  REQUIRE(eq(o1, o1));
  REQUIRE_FALSE(eq(o1, o2));
}
