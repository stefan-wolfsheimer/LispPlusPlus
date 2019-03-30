/******************************************************************************
Copyright (c) 2017-2019, Stefan Wolfsheimer

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
#include <memory>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/memory/symbol_container.h>
#include <lpp/core/object.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/compiler/scope.h>
#include <lpp/core/compiler/scope_guard.h>

using Object = Lisp::Object;
using Scope = Lisp::Scope;
using ScopeGuard = Lisp::ScopeGuard;
using Allocator = Lisp::Allocator;
using SymbolContainer = Lisp::SymbolContainer;
using Function = Lisp::Function;
using Reference = Lisp::Reference;


TEST_CASE("scope_life_cycle", "[Scope]")
{

  SymbolContainer sc;
  auto gc = std::make_shared<Allocator>();
  Object a(sc.make("a"));
  Object b(sc.make("b"));
  Object c(sc.make("c"));
  Object d(sc.make("d"));
  std::shared_ptr<Scope> root;
  std::pair<Function*, std::size_t> itr;
  Object fobj1(gc->makeRoot<Function>());
  Object fobj2(gc->makeRoot<Function>());

  // first scope
  auto s1 = std::make_shared<Scope>(root, fobj1);
  REQUIRE_FALSE(s1->getParent());
  REQUIRE(s1->getFunction() == fobj1.as<Function>());
  s1->add(a);
  s1->add(b);
  REQUIRE(s1->isSet(a, true));
  REQUIRE_FALSE(s1->isSet(c, true));
  REQUIRE(s1->isSet(a, false));
  REQUIRE_FALSE(s1->isSet(c, false));
  // check traits of function arguments
  REQUIRE(fobj1.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(fobj1.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(fobj1.as<Function>()->getArgumentTraits(1).isReference());

  // second scope
  auto s2 = std::make_shared<Scope>(s1, fobj2);
  REQUIRE(s2->getParent() == s1);
  REQUIRE(s2->getFunction() == fobj2.as<Function>());
  s2->add(c);
  s2->add(b);
  REQUIRE(s2->isSet(a, true));
  REQUIRE(s2->isSet(c, true));
  REQUIRE_FALSE(s2->isSet(a, false));
  REQUIRE(s2->isSet(c, false));

  // check traits of function arguments
  REQUIRE(fobj2.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(fobj2.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(fobj2.as<Function>()->getArgumentTraits(1).isReference());

  itr = s1->find(a, true);
  REQUIRE(itr.first == fobj1.as<Function>());
  REQUIRE(itr.second == 0);

  itr = s1->find(b, true);
  REQUIRE(itr.first == fobj1.as<Function>());
  REQUIRE(itr.second == 1);

  itr = s2->find(a, true);
  REQUIRE(itr.first == fobj1.as<Function>());
  REQUIRE(itr.second == 0);

  itr = s2->find(c, true);
  REQUIRE(itr.first == fobj2.as<Function>());
  REQUIRE(itr.second == 0);

  itr = s2->find(b, true);
  REQUIRE(itr.first == fobj2.as<Function>());
  REQUIRE(itr.second == 1);

  // create reference in parent scope
  auto sharedArgument = fobj1.as<Function>()->shareArgument(0, gc);

  REQUIRE(fobj1.as<Function>()->numArguments() == 2);
  REQUIRE(fobj1.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(fobj1.as<Function>()->getArgumentTraits(1).isReference());
  REQUIRE(sharedArgument.isA<Reference>());
  std::size_t refIndex = fobj1.as<Function>()->getArgumentTraits(0).getReferenceIndex();
  REQUIRE(fobj1.as<Function>()->atCell(refIndex).isA<Reference>());
  REQUIRE(sharedArgument.as<Reference>() == fobj1.as<Function>()->atCell(refIndex).as<Reference>());
}

TEST_CASE("scope_guard", "[Scope]")
{
  auto gc = std::make_shared<Allocator>();
  Object fobj1(gc->makeRoot<Function>());
  Object fobj2(gc->makeRoot<Function>());
  Object fobj3(gc->makeRoot<Function>());
  std::shared_ptr<Scope> root;
  std::shared_ptr<Scope> current = root;
  REQUIRE(current == root);
  {
    ScopeGuard scope_guard(current, fobj1);
    REQUIRE(current != root);
    REQUIRE(current->getParent() == root);
  }
  REQUIRE(current == root);
}
