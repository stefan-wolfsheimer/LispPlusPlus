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
#include "lisp_cons_factory.h"
#include "lisp_object.h"
#include "lisp_cons.h"
#include "lisp_vm.h"

TEST_CASE("alloc_cons_nil_nil_is_root_with_ref_count_1", "[ConsAllocator]")
{
  {
    Lisp::ConsFactory factory(8);
    Lisp::Cons * ptr = factory.make(Lisp::nil, Lisp::nil);
    REQUIRE(ptr);
    REQUIRE(ptr->getColor() == Lisp::Cons::Color::Root);
    REQUIRE(ptr->getRefCount() == 1u);
    REQUIRE(factory.numConses(Lisp::Cons::Color::Void)  == 7u);
    REQUIRE(factory.numConses(Lisp::Cons::Color::White) == 0u);
    REQUIRE(factory.numConses(Lisp::Cons::Color::Grey)  == 0u);
    REQUIRE(factory.numConses(Lisp::Cons::Color::Black) == 0u);
    REQUIRE(factory.numConses(Lisp::Cons::Color::Root)  == 1u);
  }
}

TEST_CASE("alloc_cons_cons_cons", "[ConsAllocator]")
{
  auto factory = std::make_shared<Lisp::ConsFactory>(8);
  Lisp::Vm vm(factory);
  Lisp::Cons * ptr = factory->make(vm.cons(Lisp::nil, Lisp::nil),
                                   vm.cons(Lisp::nil, Lisp::nil));
  REQUIRE(ptr);
  REQUIRE(ptr->getColor() == Lisp::Cons::Color::Root);
  REQUIRE(ptr->getRefCount() == 1u);
  REQUIRE(factory->numConses(Lisp::Cons::Color::Void)  == 5u);
  REQUIRE(factory->numConses(Lisp::Cons::Color::White) == 0u);
  REQUIRE(factory->numConses(Lisp::Cons::Color::Grey)  == 0u);
  REQUIRE(factory->numConses(Lisp::Cons::Color::Black) == 2u);
  REQUIRE(factory->numConses(Lisp::Cons::Color::Root)  == 1u);
}

