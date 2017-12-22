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
#include <limits>
#include <catch.hpp>
#include "lisp_cons_factory.h"
#include "lisp_object.h"
#include "lisp_cons.h"
#include "lisp_nil.h"

class ConsFactoryFixture : public Lisp::ConsFactory
{
public:
  static const std::size_t undef = std::numeric_limits<std::size_t>::max();

  ConsFactoryFixture(std::size_t pageSize=12) : Lisp::ConsFactory(pageSize)
  {
  }
  
  bool checkColorOfConses(Lisp::Cons::Color color,
                          const std::vector<Lisp::Cons*> conses)
  {
    for(auto cons : conses)
    {
      if(cons->getColor() != color)
      {
        return false;
      }
    }
    return true;
  }

  bool checkNumConses(Lisp::Cons::Color color, std::size_t n)
  {
    bool ret = true;
    auto nConses = numConses(color);
    auto conses = getConses(color);
    auto colorOfConsesEqual = checkColorOfConses(color, conses);
    CHECK(nConses == n);
    CHECK(conses.size() == n);
    CHECK(colorOfConsesEqual);
    return nConses == n && conses.size() == n && colorOfConsesEqual;
  }

  bool checkChildrenOfRootAndBlackConses()
  {
    using Cons = Lisp::Cons;
    using Color = Cons::Color;
    auto conses = getConses(Color::Black);
    auto root = getConses(Color::Root);
    conses.insert(conses.end(), root.begin(), root.end());
    for(auto cons : conses)
    {
      if(cons->getCarCell().isA<Cons>())
      {
        if(cons->getCarCell().as<Cons>()->getColor() == Color::White)
        {
          return false;
        }
      }
      if(cons->getCdrCell().isA<Cons>())
      {
        if(cons->getCarCell().as<Cons>()->getColor() == Color::White)
        {
          return false;
        }
      }
    }
    return true;
  }

  bool checkConses(std::size_t nRoot = undef,
                   std::size_t nBlack = undef,
                   std::size_t nGrey = undef,
                   std::size_t nWhite = undef,
                   std::size_t nVoid = undef)
  {
    bool checkNumRootConses = true;
    bool checkNumBlackConses = true;
    bool checkNumGreyConses = true;
    bool checkNumWhiteConses = true;
    bool checkNumVoidConses = true;
    if(nRoot != undef)
    {
      checkNumRootConses = checkNumConses(Color::Root, nRoot);
      CHECK(checkNumRootConses);
    }
    if(nBlack != undef)
    {
      checkNumBlackConses = checkNumConses(Color::Black, nBlack);
      CHECK(checkNumBlackConses);
    }
    if(nGrey != undef)
    {
      checkNumGreyConses = checkNumConses(Color::Grey, nGrey);
      CHECK(checkNumGreyConses);
    }
    if(nWhite != undef)
    {
      checkNumWhiteConses = checkNumConses(Color::White, nWhite);
      CHECK(checkNumWhiteConses);
    }
    if(nVoid != undef)
    {
      checkNumVoidConses = checkNumConses(Color::Void, nVoid);
      CHECK(checkNumVoidConses);
    }
    bool childrenOfRootAndBlackConses = checkChildrenOfRootAndBlackConses();
    CHECK(childrenOfRootAndBlackConses);
    return
      childrenOfRootAndBlackConses &&
      checkNumRootConses &&
      checkNumBlackConses &&
      checkNumGreyConses &&
      checkNumWhiteConses &&
      checkNumVoidConses;
  }
};

TEST_CASE("empty_cons_factory", "[ConsFactory]")
{
  ConsFactoryFixture factory(12);
  REQUIRE(factory.checkConses(0, 0, 0, 0, 0));
}

TEST_CASE("alloc_cons_nil_nil_is_root_with_ref_count_1", "[ConsFactory]")
{
  ConsFactoryFixture factory(8);
  Lisp::Cons * ptr = factory.make(Lisp::nil, Lisp::nil);
  REQUIRE(ptr);
  REQUIRE(ptr->getColor() == Lisp::Cons::Color::Root);
  REQUIRE(ptr->getRefCount() == 1u);
  REQUIRE(factory.checkConses(1u, 0, 0, 0, 7u));
}

TEST_CASE("alloc_cons_cons_cons_has_two_black_conses", "[ConsFactory]")
{
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Cons * cons = factory.make(Object(factory.make(Lisp::nil,
                                                 Lisp::nil)),
                             Object(factory.make(Lisp::nil,
                                                 Lisp::nil)));
  REQUIRE(cons);
  REQUIRE(cons->getColor() == Lisp::Cons::Color::Root);
  REQUIRE(cons->getRefCount() == 1u);
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
}

TEST_CASE("alloc_cons_cons_cons_get_car_cdr", "[ConsFactory]")
{
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Cons * cons = factory.make(Object(factory.make(Lisp::nil,
                                                 Lisp::nil)),
                             Object(factory.make(Lisp::nil,
                                                 Lisp::nil)));
  {
    auto car = cons->getCar();
    auto cdr = cons->getCdr();
    REQUIRE(car.as<Cons>()->getRefCount() == 1u);
    REQUIRE(cdr.as<Cons>()->getRefCount() == 1u);
    REQUIRE(factory.checkConses(3u, 0, 0, 0, 5u));
  }
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
}

TEST_CASE("recycle_all_unreachable_conses", "[ConsFactory]")
{
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Object cons1(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  {
    Object cons2(factory.make(Object(factory.make(Lisp::nil,
                                                  Lisp::nil)),
                              Object(factory.make(Lisp::nil,
                                                  Lisp::nil))));
    REQUIRE(factory.checkConses(2u, 4u, 0, 0, 2u));
  }
  REQUIRE(factory.checkConses(1u, 5u, 0, 0, 2u));
  factory.cycleGarbageCollector();
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
}

TEST_CASE("object_copy_constructor_cons", "[ConsFactory]")
{
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Object cons1(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  REQUIRE(cons1.isA<Cons>());
  REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
  Object cons2(cons1);
  REQUIRE(cons2.isA<Cons>());
  REQUIRE(cons2.as<Cons>() == cons1.as<Cons>());
  REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
}

TEST_CASE("object_assignment_operator_cons_set_nil", "[ConsFactory]")
{
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  using Nil = Lisp::Nil;
  ConsFactoryFixture factory(8);
  Object cons1(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  Object cons2(cons1);
  REQUIRE(cons1.as<Cons>()->getRefCount() == 2u);
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
  cons2 = Lisp::nil;
  REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
  cons1 = Lisp::nil;
  REQUIRE(cons1.isA<Nil>());
  REQUIRE(factory.checkConses(0u, 3u, 0, 0, 5u));
}

TEST_CASE("object_assignment_operator_nil_set_cons", "[ConsFactory]")
{
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Object cons1 = Lisp::nil;
  REQUIRE(factory.checkConses(0u, 0u, 0, 0, 0u));
  Object cons2(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  REQUIRE(cons2.as<Cons>()->getRefCount() == 1u);
  cons1 = cons2;
  REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
  REQUIRE(factory.checkConses(1u, 2u, 0, 0, 5u));
}

TEST_CASE("object_assignment_operator_cons_set_cons", "[ConsFactory]")
{
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  REQUIRE(factory.checkConses(0u, 0u, 0, 0, 0u));
  Object cons1(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));

  Object cons2(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  REQUIRE(factory.checkConses(2u, 4u, 0, 0, 2u));
  REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
  REQUIRE(cons2.as<Cons>()->getRefCount() == 1u);
  cons1 = cons2;
  REQUIRE(cons1.isA<Cons>());
  REQUIRE(cons1.as<Cons>() == cons2.as<Cons>());
  REQUIRE(cons1.as<Cons>()->getRefCount() == 2u);
  REQUIRE(factory.checkConses(1u, 5u, 0, 0, 2u));
}

