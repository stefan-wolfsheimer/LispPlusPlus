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
#include <lpp/core/default_env.h>
#include <lpp/core/exception.h>
#include <lpp/core/types/function.h>
#include <lpp/core/exception.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Function = Lisp::Function;
using IntegerType = Lisp::IntegerType;
using NotAList = Lisp::NotAList;
using NonMatchingArguments = Lisp::NonMatchingArguments;


TEST_CASE("lambda_constant", "[Lambda]")
{
  // (lambda (a b) 1) -> #Function
  Vm vm;
  std::size_t initStackSize = vm.stackSize(); 
  Object func = vm.compile(vm.list(vm.symbol("lambda"),
                                   vm.list(vm.symbol("a"), vm.symbol("b")),
                                   Object(1)));

  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 0);
  REQUIRE(vm.stackSize() == initStackSize);
  vm.eval(func.as<Function>());
  auto res = vm.top();
  vm.pop();
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<Function>());
  REQUIRE(res.as<Function>()->numArguments() == 2);

  // (func 2 3)
  res = vm.compileAndEval(vm.list(res, Object(2), Object(3)));
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("lambda_nested_lambdas", "[Lambda]")
{
  // (lambda (a b) a) -> #Function
  // (lambda (a b) b) -> #Function
  Vm vm;
  Object res;
  std::size_t initStackSize = vm.stackSize(); 
  Object first = vm.compileAndEval(vm.list(vm.symbol("lambda"),
                                           vm.list(vm.symbol("a"), vm.symbol("b")),
                                           vm.symbol("a")));
  REQUIRE(vm.stackSize() == initStackSize);
  Object second = vm.compileAndEval(vm.list(vm.symbol("lambda"),
                                            vm.list(vm.symbol("a"), vm.symbol("b")),
                                            vm.symbol("b")));
  REQUIRE(vm.stackSize() == initStackSize);

  REQUIRE(first.isA<Function>());
  REQUIRE(first.as<Function>()->numArguments() == 2);
  REQUIRE(second.isA<Function>());
  REQUIRE(second.as<Function>()->numArguments() == 2);

  // (first 1 2)
  res = vm.compileAndEval(vm.list(first, Object(1), Object(2)));
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);

  // (first 3, (second 1 2))
  Object select = vm.compile(vm.list(first,
                                     Object(3),
                                     vm.list(second, Object(1), Object(2))));
  REQUIRE(select.isA<Function>());
  REQUIRE(select.as<Function>()->numArguments() == 0);

  vm.eval(select.as<Function>());
  res = vm.top();
  vm.pop();
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 3);

  // (second (second 1 2) (first 3 4))
  select = vm.compile(vm.list(second,
                              vm.list(second, Object(1), Object(2)),
                              vm.list(first, Object(3), Object(4))));

  REQUIRE(select.isA<Function>());
  REQUIRE(select.as<Function>()->numArguments() == 0);

  vm.eval(select.as<Function>());
  res = vm.top();
  vm.pop();
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 3);
}
