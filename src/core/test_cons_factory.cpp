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
  const std::size_t undef = std::numeric_limits<std::size_t>::max();
  const std::size_t error = std::numeric_limits<std::size_t>::max() - 1;

  ConsFactoryFixture(std::size_t pageSize=12)
    : Lisp::ConsFactory(pageSize, 0, 0)
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

  std::size_t checkNumConses(Lisp::Cons::Color color)
  {
    auto nConses = numConses(color);
    auto conses = getConses(color);
    bool colorOfConsesEqual = true;
    if(color != Color::Free)
    {
      colorOfConsesEqual = checkColorOfConses(color, conses);
      CHECK(colorOfConsesEqual);
    }
    CHECK(nConses == conses.size());
    return (nConses == conses.size() && colorOfConsesEqual) ? nConses : error;
  }

  bool checkChildrenOfRootAndToColorConses()
  {
    using Cons = Lisp::Cons;
    using Color = Cons::Color;
    /* Todo use toColor instead of hard coded colors */
    auto conses = getConses(Color::Black);
    auto root = getConses(Color::BlackRoot);
    conses.insert(conses.end(), root.begin(), root.end());
    for(auto cons : conses)
    {
      if(cons->getCarCell().isA<Cons>())
      {
        if(cons->getCarCell().as<Cons>()->getColor() == getFromColor())
        {
          return false;
        }
      }
      if(cons->getCdrCell().isA<Cons>())
      {
        if(cons->getCarCell().as<Cons>()->getColor() == getFromColor())
        {
          return false;
        }
      }
    }
    return true;
  }

  std::vector<std::size_t> checkConses(bool checkWhiteRoot = true,
                                       bool checkGreyRoot = true,
                                       bool checkBlackRoot = true,
                                       bool checkBlack = true,
                                       bool checkGrey = true,
                                       bool checkWhite = true,
                                       bool checkVoid = true,
                                       bool checkFree = true)
  {
    std::vector<std::size_t> ret({
        checkWhiteRoot ? checkNumConses(Color::WhiteRoot) : undef,
        checkGreyRoot  ? checkNumConses(Color::GreyRoot) : undef,
        checkBlackRoot ? checkNumConses(Color::BlackRoot) : undef,
        checkBlack     ? checkNumConses(Color::Black) : undef,
        checkGrey      ? checkNumConses(Color::Grey) : undef,
        checkWhite     ? checkNumConses(Color::White) : undef,
        checkVoid      ? checkNumConses(Color::Void) : undef,
        checkFree      ? checkNumConses(Color::Free) : undef});
    bool childrenOfRootAndToColorConses = checkChildrenOfRootAndToColorConses();
    CHECK(childrenOfRootAndToColorConses);
    if(!childrenOfRootAndToColorConses)
    {
      ret.push_back(undef);
    }
    return ret;
  }
};

TEST_CASE("empty_cons_factory", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  ConsFactoryFixture factory(12);
  REQUIRE(factory.checkConses() == V({ 0, 0, 0, 0, 0, 0, 0, 0}));
}

TEST_CASE("alloc_cons_nil_nil_is_root_with_ref_count_1", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  using Cons = Lisp::Cons;
  ConsFactoryFixture factory(8);
  Cons * ptr = factory.make(Lisp::nil, Lisp::nil);
  REQUIRE(ptr);
  REQUIRE(ptr->getColor() == factory.getFromRootColor());
  REQUIRE(ptr->getRefCount() == 1u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 0, 0, 0, 7, 0}));
}

TEST_CASE("alloc_cons_cons_cons_has_two_black_conses", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Cons * cons = factory.make(Object(factory.make(Lisp::nil,
                                                 Lisp::nil)),
                             Object(factory.make(Lisp::nil,
                                                 Lisp::nil)));
  REQUIRE(cons);
  REQUIRE(cons->getColor() == factory.getFromRootColor());
  REQUIRE(cons->getRefCount() == 1u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
}

TEST_CASE("alloc_cons_cons_cons_get_car_cdr", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
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
    REQUIRE(factory.checkConses() == V({3, 0, 0, 0, 0, 0, 5, 0}));
  }
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
}

TEST_CASE("recycle_all_unreachable_conses", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
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
    REQUIRE(factory.checkConses() == V({2,0, 0, 4, 0, 0, 2, 0}));
  }
  REQUIRE(factory.checkConses() == V({1, 0, 0, 5, 0, 0, 2, 0}));
  factory.cycleGarbageCollector();
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
}

TEST_CASE("object_copy_constructor_cons", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
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
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
}

TEST_CASE("object_assignment_operator_cons_set_nil", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
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
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
  cons2 = Lisp::nil;
  REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
  cons1 = Lisp::nil;
  REQUIRE(cons1.isA<Nil>());
  REQUIRE(factory.checkConses() == V({0, 0, 0, 3, 0, 0, 5, 0}));
}

TEST_CASE("object_assignment_operator_nil_set_cons", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  Object cons1 = Lisp::nil;
  REQUIRE(factory.checkConses() == V({0, 0, 0, 0, 0, 0, 0, 0}));
  Object cons2(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  REQUIRE(cons2.as<Cons>()->getRefCount() == 1u);
  cons1 = cons2;
  REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
}

TEST_CASE("object_assignment_operator_cons_set_cons", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  using Cons = Lisp::Cons;
  using Object = Lisp::Object;
  ConsFactoryFixture factory(8);
  REQUIRE(factory.checkConses() == V({0, 0, 0, 0, 0, 0, 0, 0}));
  Object cons1(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));

  Object cons2(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  REQUIRE(factory.checkConses() == V({2, 0, 0, 4, 0, 0, 2, 0}));
  REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
  REQUIRE(cons2.as<Cons>()->getRefCount() == 1u);
  cons1 = cons2;
  REQUIRE(cons1.isA<Cons>());
  REQUIRE(cons1.as<Cons>() == cons2.as<Cons>());
  REQUIRE(cons1.as<Cons>()->getRefCount() == 2u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 5, 0, 0, 2, 0}));
}

