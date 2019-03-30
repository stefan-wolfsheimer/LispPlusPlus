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
#include <lpp/core/cell.h>
#include <lpp/core/object.h>
#include <lpp/core/vm.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/types/symbol.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Cell = Lisp::Cell;
using Function = Lisp::Function;
using IntegerType = Lisp::IntegerType;
using Reference = Lisp::Reference;
using Symbol = Lisp::Symbol;

TEST_CASE("eval_value", "[Value]")
{
  Vm vm;
  std::size_t stackSize = vm.stackSize();
  auto func = vm.compile(Object(1));
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  REQUIRE(stackSize == vm.stackSize());
  vm.eval(func.as<Function>());
  auto res = vm.top();
  vm.pop();
  REQUIRE(stackSize == vm.stackSize());
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1u);
}

TEST_CASE("eval_reference", "[Value]")
{
  Vm vm;
  Object a(vm.make<Symbol>("a"));
  Object refA(vm.reference(a, Cell(1)));
  REQUIRE(refA.isA<Reference>());
  std::size_t stackSize = vm.stackSize();
  auto func = vm.compile(refA);
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  REQUIRE(stackSize == vm.stackSize());
  vm.eval(func.as<Function>());
  auto res = vm.top();
  vm.pop();
  REQUIRE(stackSize == vm.stackSize());
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1u);
}
