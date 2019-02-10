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
#include <catch.hpp>
#include <lpp/core/vm.h>
#include <lpp/core/opcode.h>
 //#include <lpp/core/types/cons.h>
#include <lpp/core/types/function.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
//using Cons = Lisp::Cons;
//using Nil = Lisp::Nil;
using Function = Lisp::Function;
using IntegerType = Lisp::IntegerType;


TEST_CASE("eval_lookup", "[Define]")
{
  using Undefined = Lisp::Undefined;
  Vm vm;
  std::size_t stackSize = vm.stackSize();
  auto func = vm.compile(vm.symbol("a"));
  REQUIRE(func.isA<Function>());
  REQUIRE(func.getRefCount() == 1u);
  // @todo exception if unbound
  vm.eval(func.as<Function>());
  auto res = vm.top();
  vm.pop();
  REQUIRE(res.isA<Undefined>());
  REQUIRE(stackSize == vm.stackSize());
}

TEST_CASE("eval_define", "[Vm]")
{
  using Undefined = Lisp::Undefined;
  Vm vm;
  vm.getGarbageCollector()->disableCollector();
  std::size_t stackSize = vm.stackSize();
  Function * f;
  {
    Object func(std::move(vm.compile(vm.list(vm.symbol("define"),
                                             vm.symbol("a"),
                                             Object(1)))));
    REQUIRE(func.getRefCount() == 1u);
    REQUIRE(func.isA<Function>());
    vm.eval(func.as<Function>());
    vm.pop();
    vm.getGarbageCollector()->cycle();
    f = func.as<Function>();
  }
  auto res = vm.find("a");
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
  {
    auto func = vm.compile(vm.symbol("a"));
    vm.eval(func.as<Function>());
    auto res = vm.top();
    vm.pop();
    REQUIRE(res.isA<IntegerType>());
    REQUIRE(res.as<IntegerType>() == 1);
  }
}
