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
#include <memory>
#include <catch.hpp>
#include <lpp/core/vm.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/continuation.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Cell = Lisp::Cell;
using Allocator = Lisp::Allocator;
using Color = Lisp::Color;

// types
using Nil = Lisp::Nil;
using IntegerType = Lisp::IntegerType;
using Cons = Lisp::Cons;
using Symbol = Lisp::Symbol;
using Function = Lisp::Function;
using Continuation = Lisp::Continuation;

template<typename T>
bool checkForEachCollectible(std::shared_ptr<Allocator> alloc, T * obj)
{
  bool found = false;
  alloc->forEachCollectible([&found, obj](const Cell & cell) {
      if(cell.as<T>() == obj)
      {
        found = true;
      }
    });
  return found;
}

TEST_CASE("continuation_life", "[Continuation]")
{
  Vm vm;
  std::shared_ptr<Allocator> alloc = vm.getAllocator();
  Function * funcPtr;
  Cons * cons1Ptr;
  Cons * cons2Ptr;
  alloc->setGarbageSteps(1);
  alloc->setRecycleSteps(1);
  alloc->disableCollector();
  alloc->disableRecycling();
  Object cont;
  Object func = vm.make<Function>();
  {
    funcPtr = func.as<Function>();
    REQUIRE(funcPtr);
    cont = vm.make<Continuation>(func.as<Function>());
    Object cons1 = vm.make<Cons>(Lisp::nil, Lisp::nil);
    Object cons2 = vm.make<Cons>(cons1, Lisp::nil);
    cons1Ptr = cons1.as<Cons>();
    cons2Ptr = cons2.as<Cons>();
    cont.as<Continuation>()->push(cons1);
    cont.as<Continuation>()->push(cons2);
  }
  REQUIRE(cont.isA<Continuation>());
  REQUIRE(cont.getRefCount() == 1u);
  REQUIRE(funcPtr->getRefCount() == 1u);
  func = Lisp::nil;
  REQUIRE(funcPtr->getRefCount() == 0u);
  REQUIRE(checkForEachCollectible(alloc, funcPtr));
  REQUIRE(checkForEachCollectible(alloc, cons1Ptr));
  REQUIRE(checkForEachCollectible(alloc, cons2Ptr));
  REQUIRE(checkForEachCollectible(alloc, cont.as<Continuation>()));
  alloc->cycle();
  REQUIRE(funcPtr->getColor() != Color::Grey);
  REQUIRE(cons1Ptr->getColor() != Color::Grey);
  REQUIRE(cons2Ptr->getColor() != Color::Grey);
  alloc->enableCollector();

  alloc->step();
  REQUIRE(funcPtr->getColor() == Color::Grey);
  REQUIRE(cons1Ptr->getColor() == Color::White);
  REQUIRE(cons2Ptr->getColor() == Color::White);

  alloc->step();
  REQUIRE(funcPtr->getColor() == Color::Grey);
  REQUIRE(cons1Ptr->getColor() == Color::Grey);
  REQUIRE(cons2Ptr->getColor() == Color::White);

  alloc->step();
  REQUIRE(funcPtr->getColor() == Color::Grey);
  REQUIRE(cons1Ptr->getColor() == Color::Black);
  REQUIRE(cons2Ptr->getColor() == Color::Grey);

  alloc->step();
  REQUIRE(funcPtr->getColor() == Color::Black);
  REQUIRE(cons1Ptr->getColor() == Color::Black);
  REQUIRE(cons2Ptr->getColor() == Color::Black);

  alloc->disableCollector();
  cont = Lisp::nil;
  alloc->enableCollector();
  alloc->step();
  alloc->step();
  alloc->step();
  alloc->disableCollector();
  REQUIRE(alloc->numDisposedCollectible() == 4);
  alloc->enableRecycling();
  alloc->recycle();
  REQUIRE(alloc->numDisposedCollectible() == 3);
  alloc->recycle();
  REQUIRE(alloc->numDisposedCollectible() == 2);
  alloc->recycle();
  REQUIRE(alloc->numDisposedCollectible() == 1);
  alloc->recycle();
  alloc->recycle();
  alloc->recycle();
  REQUIRE(alloc->numDisposedCollectible() == 0);
  alloc->disableRecycling();
}

TEST_CASE("continuation_eval", "[Continuation]")
{
  Vm vm;
  Object func = vm.make<Function>();
  Object cont = vm.make<Continuation>(func.as<Function>());
  REQUIRE(cont.as<Continuation>()->stackSize() == 1u);
  REQUIRE(cont.as<Continuation>()->eval().as<Function>() == func.as<Function>());
}

TEST_CASE("continuation_returnv", "[Continuation]")
{
  Vm vm;
  Object func = vm.make<Function>();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(1));
  Object cont = vm.make<Continuation>(func.as<Function>());
  Object res(cont.as<Continuation>()->eval());
  REQUIRE(cont.as<Continuation>()->stackSize() == 1u);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("continuation_returns", "[Continuation]")
{
  Vm vm;
  Object func = vm.make<Function>();
  func.as<Function>()->addRETURNS(1);
  Object cont = vm.make<Continuation>(func.as<Function>());
  cont.as<Continuation>()->push(Cell(1));
  cont.as<Continuation>()->push(Cell(2));
  Object res(cont.as<Continuation>()->eval());
  REQUIRE(cont.as<Continuation>()->stackSize() == 1u);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 2);
}

TEST_CASE("continuation_returnl", "[Continuation]")
{
  Vm vm;
  Object func = vm.make<Function>();
  vm.define("a", vm.make<IntegerType>(3));
  func.as<Function>()->addRETURNL(vm.make<Symbol>("a"));
  Object cont = vm.make<Continuation>(func.as<Function>());
  Object res(cont.as<Continuation>()->eval());
  REQUIRE(cont.as<Continuation>()->stackSize() == 1u);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 3);
}

TEST_CASE("continuation_incret", "[Continuation]")
{
  Vm vm;
  Object func = vm.make<Function>();
  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(1));

  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(2));

  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(3));

  Object cont = vm.make<Continuation>(func.as<Function>());
  Object res(cont.as<Continuation>()->eval());
  REQUIRE(cont.as<Continuation>()->stackSize() == 4u);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 3);
}

TEST_CASE("continuation_defines", "[Continuation]")
{
  Vm vm;
  Object func = vm.make<Function>();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(10));
  func.as<Function>()->addDEFINES(vm.make<Symbol>("a"));
  Object cont = vm.make<Continuation>(func.as<Function>());
  Object res(cont.as<Continuation>()->eval());
  REQUIRE(cont.as<Continuation>()->stackSize() == 1u);
  res = vm.find("a");
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 10);
}

TEST_CASE("continuation_funcall", "[Continuation]")
{
  Vm vm;
  Object lambda = vm.make<Function>();
  lambda.as<Function>()->addRETURNV(vm.make<IntegerType>(10));
  lambda.as<Function>()->addArgument(vm.make<Symbol>("a"));
  lambda.as<Function>()->addArgument(vm.make<Symbol>("b"));
  lambda.as<Function>()->addArgument(vm.make<Symbol>("c"));
  std::cout << "XXXXXXXXXXXX" << std::endl;
  lambda.as<Function>()->disassemble(std::cout);
  std::cout << "YYYYYYYYYYYY" << std::endl;

  Object func = vm.make<Function>();
  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(lambda);
  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(10));
  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(20));
  func.as<Function>()->addINCRET();
  func.as<Function>()->addRETURNV(vm.make<IntegerType>(30));
  func.as<Function>()->addFUNCALL(3);
  func.as<Function>()->disassemble(std::cout);
  std::cout << "YYYYYYYYYYYY" << std::endl;

  Object cont = vm.make<Continuation>(func.as<Function>());
  Object res(cont.as<Continuation>()->eval());
  REQUIRE(res.as<IntegerType>() == 10);
  REQUIRE(cont.as<Continuation>()->stackSize() == 2u);
}
