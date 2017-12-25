/******************************************************************************
Copyright (c) 2017, Stefan Wolfsheimer

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
#include "lisp_cons.h"
#include "lisp_vm.h"

TEST_CASE("cons_allocator_color_has_size_1", "[Cons]")
{
  REQUIRE(sizeof(Lisp::Cons::Color) == 1u);
}

TEST_CASE("nil_is_not_a_cons", "[Cons]")
{
  REQUIRE_FALSE(Lisp::nil.isA<Lisp::Cons>());
}

TEST_CASE("nil_as_cons_is_pullptr", "[Cons]")
{
  REQUIRE(Lisp::nil.as<Lisp::Cons>() == nullptr);
}

TEST_CASE("cons_is_a_cons", "[Cons]")
{
  Lisp::Vm vm;
  auto obj = vm.cons(Lisp::nil, Lisp::nil);
  REQUIRE(obj.isA<Lisp::Cons>());
}

TEST_CASE("cons_as_cons_is_cons", "[Cons]")
{
  Lisp::Vm vm;
  auto obj = vm.cons(Lisp::nil, Lisp::nil);
  REQUIRE(obj.as<Lisp::Cons>());
}

TEST_CASE("cons_has_refcount_1", "[Cons]")
{
  Lisp::Vm vm;
  auto obj = vm.cons(Lisp::nil, Lisp::nil);
  REQUIRE(obj.as<Lisp::Cons>()->getRefCount() == 1u);
}

TEST_CASE("cons_is_root", "[Cons]")
{
  using Cons = Lisp::Cons;
  using Color = Cons::Color;
  Lisp::Vm vm;
  auto obj = vm.cons(Lisp::nil, Lisp::nil);
  REQUIRE(obj.as<Cons>()->isRoot());
}

