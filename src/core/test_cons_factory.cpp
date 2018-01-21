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
#include <limits>
#include <vector>
#include <unordered_set>
#include <catch.hpp>

#include "lisp_cons_factory.h"
#include "lisp_cons.h"
#include "lisp_object.h"
#include "lisp_nil.h"

// helper types
using ConsFactory = Lisp::ConsFactory;
using SharedConsFactory = std::shared_ptr<ConsFactory>;
using Cons = Lisp::Cons;
using Color = ConsFactory::Color;
using Object = Lisp::Object;
using Nil = Lisp::Nil;

// helper constants
static const std::size_t undef = std::numeric_limits<std::size_t>::max();
static const std::size_t error = std::numeric_limits<std::size_t>::max() - 1;

// helper functions
static SharedConsFactory makeFactory(std::size_t pageSize=12);
static std::size_t getNumConses(SharedConsFactory factory, Color color);
static std::size_t getNumConses(SharedConsFactory factory);
static bool checkColorOfConses(SharedConsFactory factory, Color color, const std::vector<Cons*> conses);
static std::pair<Color, std::size_t> operator==(Color, std::size_t);
static bool checkConses(SharedConsFactory factory, const std::vector<std::pair<Color, std::size_t> > & conses);
static bool checkConses(SharedConsFactory factory, std::size_t total, const std::vector<std::pair<Color, std::size_t> > & conses);
static std::unordered_set<Cons*> setOfConses(const std::vector<Cons*> & conses);
static std::unordered_set<Cons*> setOfConses(Cons * c1 = nullptr,
                                             Cons * c2 = nullptr,
                                             Cons * c3 = nullptr,
                                             Cons * c4 = nullptr,
                                             Cons * c5 = nullptr,
                                             Cons * c6 = nullptr,
                                             Cons * c7 = nullptr,
                                             Cons * c8 = nullptr,
                                             Cons * c9 = nullptr,
                                             Cons * c10 = nullptr);
static std::unordered_set<Cons*> getReachableConses(SharedConsFactory factory);

SCENARIO("no cons allocated", "[ConsFactory]")
{
  GIVEN("An empty cons factory")
  {
    auto factory = makeFactory(8);
    WHEN("there is no cons allocated")
    {
      THEN("there is no cons for any color")
      {
        REQUIRE(checkConses(factory, 0u, {Color::Void==0u}));
        REQUIRE(getNumConses(factory, Color::Void) == 0u);
        REQUIRE(getNumConses(factory, Color::White) == 0u);
        REQUIRE(getNumConses(factory, Color::Grey) == 0u);
        REQUIRE(getNumConses(factory, Color::Black) == 0u);
        REQUIRE(getNumConses(factory, Color::WhiteRoot) == 0u);
        REQUIRE(getNumConses(factory, Color::GreyRoot) == 0u);
        REQUIRE(getNumConses(factory, Color::BlackRoot) == 0u);
        REQUIRE(getNumConses(factory, Color::Free) == 0u);
      }
    }
  }
}

SCENARIO("a cons without children", "[ConsFactory]")
{
  GIVEN("A cons factory")
  {
    auto factory = makeFactory(8);
    WHEN("there is a cons without children is allocated")
    {
      Cons * pcons = factory->make(Lisp::nil, Lisp::nil);
      THEN("the new cons is in root set and has from-color")
      {
        REQUIRE(pcons);
        REQUIRE(pcons->getColor() == factory->getFromRootColor());
      }
      THEN("the new cons has ref-count 1")
      {
        REQUIRE(pcons->getRefCount() == 1u);
      }
      THEN("there is 1 root with from-color and 7 void conses")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, Color::Void == 7u}));
      }
    }
  }
}

SCENARIO("a cons with 2 children", "[ConsFactory]")
{
  /*
              o
             / \
            o   o 
  */
  GIVEN("A cons factory")
  {
    auto factory = makeFactory(8);
    WHEN("There is a cons with 2 children")
    {
      Cons * pcons = factory->make(Object(factory->make(Lisp::nil,
                                                        Lisp::nil)),
                                   Object(factory->make(Lisp::nil,
                                                        Lisp::nil)));
      THEN("The new cons is root and has from-root-color")
      {
        REQUIRE(pcons->getColor() == factory->getFromRootColor());
      }
      THEN("Children of new cons has to-color")
      {
        REQUIRE(pcons->getCarCell().as<Cons>()->getColor() == factory->getToColor());
        REQUIRE(pcons->getCdrCell().as<Cons>()->getColor() == factory->getToColor());
      }
      THEN("the new cons has ref-count 1")
      {
        REQUIRE(pcons->getRefCount() == 1u);
      }
      THEN("there is 1 root with from-color, 2 with from-color and 5 void conses")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getToColor() == 2u, Color::Void == 5u}));
      }
      THEN("reachable conses are the cons and its children")
      {
        REQUIRE(getReachableConses(factory) == setOfConses(pcons,
                                                           pcons->getCarCell().as<Cons>(),
                                                           pcons->getCdrCell().as<Cons>()));
      }
      WHEN("two references to the children are allocated")
      {
        {
          auto car = pcons->getCar();
          auto cdr = pcons->getCdr();
          THEN("reference count is 1")
          {
            REQUIRE(car.as<Cons>()->getRefCount() == 1u);
            REQUIRE(cdr.as<Cons>()->getRefCount() == 1u);
          }
          THEN("there are 3 conses with root-from-color and 5 void conses")
          {
            REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==3u, Color::Void == 5u}));
          }
        }
        THEN("finally there is cons with root-from-clor and 2 conses with to-color")
        {
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getToColor() == 2u, Color::Void == 5u}));
        }
      }
    }
  }
}

SCENARIO("3 conses with 4 children", "[ConsFactory]")
{
  /*
         o        o    o
        / \      / \  / \
       o   o    o   o
  */
  GIVEN("A cons factory")
  {
    auto factory = makeFactory(8);
    Object cons1(factory->make(Object(factory->make(Lisp::nil,
                                                    Lisp::nil)),
                               Object(factory->make(Lisp::nil,
                                                    Lisp::nil))));
    WHEN("there are 2 conses with 4 children")
    {
      Object cons3;
      Object cons2(factory->make(Object(factory->make(Lisp::nil,
                                                      Lisp::nil)),
                                 Object(factory->make(Lisp::nil,
                                                      Lisp::nil))));
      THEN("there are 2 conses with from-root-color and 4 conses with to-color")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u, factory->getToColor() == 4u, Color::Void == 2u}));
      }
      WHEN("there are 3 conses with 4 children")
      {
        cons3 = Object(factory->make(cons2.as<Cons>()->getCar(), Lisp::nil));
        THEN("there are 3 conses with from-root-color and 4 conses with to-color")
        {
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==3u, factory->getToColor() == 4u, Color::Void == 1u}));
        }
        THEN("all children of cons1, cons2 and cons3 are reachable")
        {
          REQUIRE(getReachableConses(factory) == setOfConses(cons1.as<Cons>(),
                                                             cons1.as<Cons>()->getCarCell().as<Cons>(),
                                                             cons1.as<Cons>()->getCdrCell().as<Cons>(),
                                                             cons2.as<Cons>(),
                                                             cons2.as<Cons>()->getCarCell().as<Cons>(),
                                                             cons2.as<Cons>()->getCdrCell().as<Cons>(),
                                                             cons3.as<Cons>(),
                                                             cons3.as<Cons>()->getCarCell().as<Cons>()));
        }
      }
    }
    WHEN("there are 2 conses with 3 children")
    {
      /*
         o            o
        / \          / \
       o   o        o
      */
      Object cons3;
      {
        Object cons2(factory->make(Object(factory->make(Lisp::nil,
                                                        Lisp::nil)),
                                   Object(factory->make(Lisp::nil,
                                                        Lisp::nil))));
        cons3 = Object(factory->make(cons2.as<Cons>()->getCar(), Lisp::nil));
      }
      THEN("there are 2 root conses and 3 leafes")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u, factory->getToColor() == 5u, Color::Void == 1u}));
        REQUIRE(getReachableConses(factory) == setOfConses(cons1.as<Cons>(),
                                                           cons1.as<Cons>()->getCarCell().as<Cons>(),
                                                           cons1.as<Cons>()->getCdrCell().as<Cons>(),
                                                           cons3.as<Cons>(),
                                                           cons3.as<Cons>()->getCarCell().as<Cons>()));
      }
      WHEN("cycle garbage collector")
      {
        REQUIRE(factory->getFromColor() == Color::White);
        factory->cycleGarbageCollector();
        REQUIRE(factory->getFromColor() == Color::White);
        THEN("there are 2 from-root-color and 3 from-color conses")
        {
          //@todo: check this:
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u, factory->getToColor() == 3u, Color::Void == 3u}));
        }
        THEN("there are x reachable conses")
        {
          REQUIRE(getReachableConses(factory) == setOfConses(factory->getConses(Color::White, Color::Free)));
        }
      }
    }
  }
}

SCENARIO("copy cons object with object copy constructor", "[ConsFactory]")
{
  GIVEN("A cons with 2 children")
  {
    auto factory = makeFactory(8);
    Object cons1(factory->make(Object(factory->make(Lisp::nil,
                                                    Lisp::nil)),
                               Object(factory->make(Lisp::nil,
                                                    Lisp::nil))));
    WHEN("there is no copy")
    {
      THEN("ref count is 1")
      {
        REQUIRE(cons1.isA<Cons>());
        REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
      }
    }

    WHEN("there is a copy of the cons")
    {
      Object cons2(cons1);
      THEN("copy is a cons")
      {
        REQUIRE(cons2.isA<Cons>());
      }
      THEN("pointers are equal")
      {
        REQUIRE(cons2.as<Cons>() == cons1.as<Cons>());
      }
      THEN("ref count is 2")
      {
        REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
      }
      THEN("there is one root cons and 2 leafs")
      {
        REQUIRE(getNumConses(factory) == 8u);
        REQUIRE(getNumConses(factory, factory->getFromRootColor()) == 1u);
        //@todo: check this:
        REQUIRE(getNumConses(factory, factory->getToColor()) == 2u);
        REQUIRE(getNumConses(factory, Color::Void) == 5u);
      }
      WHEN("copy is unset")
      {
        cons2 = Lisp::nil;
        THEN("ref count of cons1 is 1")
        {
          REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
        }
        THEN("there is one root cons and 2 leafs")
        {
          //@todo: check this:
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getToColor() == 2u, Color::Void == 5u}));
        }
        WHEN("both conses are unset")
        {
          cons1 = Lisp::nil;
          REQUIRE(cons1.isA<Nil>());
          THEN("there is no root and 3 leaf conses")
          {
            //@todo: check this:
            REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==0u, factory->getToColor() == 3u, Color::Void == 5u}));
          }
          WHEN("cycle garbage location")
          {
            factory->cycleGarbageCollector();
            THEN("there is no root and 3 leaf conses")
            {
              REQUIRE(checkConses(factory, 8u, { Color::Void == 8u}));
            }
          }
        }
      }
    }
  }
}

SCENARIO("copy cons object with object assignement operator", "[ConsFactory]")
{
  GIVEN("conses with 4 children")
  {
    auto factory = makeFactory(8);
    Object cons1(factory->make(Object(factory->make(Lisp::nil,
                                                    Lisp::nil)),
                               Object(factory->make(Lisp::nil,
                                                    Lisp::nil))));
    Object cons2(factory->make(Object(factory->make(Lisp::nil,
                                                    Lisp::nil)),
                               Object(factory->make(Lisp::nil,
                                                    Lisp::nil))));
    WHEN("cons1 is set to cons2")
    {
      REQUIRE(cons2.as<Cons>()->getRefCount() == 1u);
      cons1 = cons2;
      THEN("ref count is 2")
      {
        REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
      }
      THEN("there is 1 root cons and 5 leaf")
      {
        //@todo: check this:
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getToColor() == 5u, Color::Void == 2u}));
        WHEN("cycle garbage collector")
        {
          factory->cycleGarbageCollector();
          THEN("there is no root and 3 leaf conses")
          {
            REQUIRE(getNumConses(factory) == 8u);
            REQUIRE(getNumConses(factory, factory->getFromRootColor()) == 1u);
            //@todo: check this:
            REQUIRE(getNumConses(factory, factory->getToColor()) == 2u);
            REQUIRE(getNumConses(factory, Color::Void) == 5u);
          }
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////
/// implementation
//////////////////////////////////////////////////////////
static SharedConsFactory makeFactory(std::size_t pageSize)
{
  return std::make_shared<ConsFactory>(pageSize, 0, 0);
}

static bool checkColorOfConses(SharedConsFactory factory, Color color, const std::vector<Cons*> conses)
{
  for(auto cons : conses)
  {
    if(cons->getColor() != color)
    {
      return false;
    }
  }
  if(color == factory->getToColor())
  {
    for(auto cons : conses)
    {
      if(cons->getCarCell().isA<Cons>())
      {
        if(cons->getCarCell().as<Cons>()->getColor() == factory->getFromColor())
        {
          return false;
        }
      }
      if(cons->getCdrCell().isA<Cons>())
      {
        if(cons->getCarCell().as<Cons>()->getColor() == factory->getFromColor())
        {
          return false;
        }
      }
    }
  }
  return true;
}

static std::size_t getNumConses(SharedConsFactory factory)
{
  std::vector<Color> v({Color::Void,
                        Color::White,
                        Color::Grey,
                        Color::Black,
                        Color::WhiteRoot,
                        Color::GreyRoot,
                        Color::BlackRoot,
                        Color::Free});
  std::size_t ret = 0;
  for(auto color : v)
  {
    auto n = getNumConses(factory, color);
    if(n == error || n == undef)
    {
      return n;
    }
    ret += n;
  }
  return ret;
}

static std::size_t getNumConses(SharedConsFactory factory, Color color)
{
  auto nConses = factory->numConses(color);
  auto conses = factory->getConses(color);
  bool colorOfConsesEqual = true;
  if(color != Color::Free)
  {
    colorOfConsesEqual = checkColorOfConses(factory, color, conses);
    CHECK(colorOfConsesEqual);
  }
  CHECK(nConses == conses.size());
  return (nConses == conses.size() && colorOfConsesEqual) ? nConses : error;
}

std::pair<Color, std::size_t> operator==(Color color, std::size_t n)
{
  return std::pair<Color, std::size_t>(color, n);
}

bool checkConses(SharedConsFactory factory, std::size_t total, const std::vector<std::pair<Color, std::size_t> > & conses)
{
  std::size_t num_conses = getNumConses(factory);
  CHECK(num_conses == total);
  if(num_conses != total)
  {
    return false;
  }
  return checkConses(factory, conses);
}


bool checkConses(SharedConsFactory factory, const std::vector<std::pair<Color, std::size_t> > & conses)
{
  for(auto p : conses)
  {
    std::size_t num_conses = getNumConses(factory, p.first);
    CHECK(num_conses == p.second);
    if(num_conses != p.second)
    {
      return false;
    }
  }
  return true;
}

std::unordered_set<Cons*> setOfConses(const std::vector<Cons*> & conses)
{
  return std::unordered_set<Cons*>(conses.begin(), conses.end());
}

std::unordered_set<Cons*> setOfConses(Cons * c1,
                                      Cons * c2,
                                      Cons * c3,
                                      Cons * c4,
                                      Cons * c5,
                                      Cons * c6,
                                      Cons * c7,
                                      Cons * c8,
                                      Cons * c9,
                                      Cons * c10)
{
  std::unordered_set<Cons*> ret;
  if(c1) ret.insert(c1);
  if(c2) ret.insert(c2);
  if(c3) ret.insert(c3);
  if(c4) ret.insert(c4);
  if(c5) ret.insert(c5);
  if(c6) ret.insert(c6);
  if(c7) ret.insert(c7);
  if(c8) ret.insert(c8);
  if(c9) ret.insert(c9);
  if(c10) ret.insert(c10);
  return ret;
}

static std::unordered_set<Cons*> getReachableConses(SharedConsFactory factory)
{
  return setOfConses(factory->getReachableConses());
}
