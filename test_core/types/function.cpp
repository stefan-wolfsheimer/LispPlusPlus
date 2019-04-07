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
#include <lpp/core/types/function.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/types/symbol.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Function = Lisp::Function;
using Symbol = Lisp::Symbol;
using Reference = Lisp::Reference;
using IntegerType = Lisp::IntegerType;

TEST_CASE("function", "[Function]")
{
  Vm vm;
  Object f1 = vm.make<Function>();
  Object f2 = vm.make<Function>();
  f1.as<Function>()->addArgument(vm.make<Symbol>("a"));
  f1.as<Function>()->addArgument(vm.make<Symbol>("b"));
  REQUIRE(f1.as<Function>()->numArguments() == 2);
  REQUIRE(f1.as<Function>()->getArgumentPos(vm.make<Symbol>("a")) == 0);
  REQUIRE(f1.as<Function>()->getArgumentPos(vm.make<Symbol>("b")) == 1);
  REQUIRE(f1.as<Function>()->getArgumentPos(vm.make<Symbol>("c")) == Function::notFound);
  REQUIRE_FALSE(f1.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(f1.as<Function>()->getArgumentTraits(1).isReference());

  Object sa = f1.as<Function>()->shareArgument(0);
  REQUIRE(f1.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(f1.as<Function>()->getArgumentTraits(1).isReference());
  std::size_t refIndex = f1.as<Function>()->getArgumentTraits(0).getReferenceIndex();
  REQUIRE(f1.as<Function>()->atCell(refIndex).isA<Reference>());
  REQUIRE(sa.as<Reference>() == f1.as<Function>()->atCell(refIndex).as<Reference>());
  sa.as<Reference>()->setValue(vm.make<IntegerType>(12));
  REQUIRE(f1.as<Function>()->getValue(refIndex).isA<IntegerType>());
  REQUIRE(f1.as<Function>()->getValue(refIndex).as<IntegerType>() == 12);
}
