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
#include <core/lisp_vm.h>
#include <core/types/cons.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Cons = Lisp::Cons;
using Nil = Lisp::Nil;

TEST_CASE("is_debug_enabled", "[Vm]")
{
  Vm vm;
  REQUIRE(vm.withDebug);
}

TEST_CASE("vm_move_cons", "[Vm]")
{
  Vm vm;
  Object cons = std::move(vm.cons(Lisp::nil, Lisp::nil));
  REQUIRE(cons.isA<Cons>());
  REQUIRE(cons.as<Cons>()->getRefCount() == 1u);
}

TEST_CASE("vm_cons_does_not_create_temporary_objects", "[Vm]")
{
  Vm vm;
  {
    Object cons = vm.cons(Lisp::nil, Lisp::nil);
    REQUIRE(cons.isA<Cons>());
  }
}

TEST_CASE("vm_list_does_not_create_temporary_objects", "[Vm]")
{
  Vm vm;
  {
    Object lst = vm.list();
    REQUIRE(lst.isA<Nil>());
  }
  {
    Object lst = vm.list(Lisp::nil);
    REQUIRE(lst.isA<Cons>());
  }
  {
    Object lst = vm.list(Lisp::nil, Lisp::nil, Lisp::nil, Lisp::nil);
    REQUIRE(lst.isA<Cons>());
    REQUIRE(lst.as<Cons>()
            ->getCar().isA<Nil>());
    REQUIRE(lst.as<Cons>()
            ->getCdr().isA<Cons>());
    REQUIRE(lst.as<Cons>()
            ->getCdr().as<Cons>()
            ->getCar().isA<Nil>());
    REQUIRE(lst.as<Cons>()
            ->getCdr().as<Cons>()
            ->getCdr().as<Cons>()
            ->getCar().isA<Nil>());
    REQUIRE(lst.as<Cons>()
            ->getCdr().as<Cons>()
            ->getCdr().as<Cons>()
            ->getCdr().as<Cons>()
            ->getCar().isA<Nil>());
  }
}

TEST_CASE("push_does_not_create_temporary_objects", "[Vm]")
{
  Vm vm;
  vm.push(Lisp::nil);
  vm.push(vm.cons(Lisp::nil, Lisp::nil));
}

