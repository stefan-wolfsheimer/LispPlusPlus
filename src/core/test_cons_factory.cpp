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
#include <catch.hpp>

#include "lisp_cons_factory.h"
#include "lisp_cons.h"
#include "lisp_object.h"

// helper types
using ConsFactory = Lisp::ConsFactory;
using SharedConsFactory = std::shared_ptr<ConsFactory>;
using Cons = Lisp::Cons;
using Color = ConsFactory::Color;
using Object = Lisp::Object;

// helper constants
static const std::size_t undef = std::numeric_limits<std::size_t>::max();
static const std::size_t error = std::numeric_limits<std::size_t>::max() - 1;

// helper functions
static SharedConsFactory makeFactory(std::size_t pageSize=12);
static std::size_t getNumConses(SharedConsFactory factory, Color color);
static std::size_t getNumConses(SharedConsFactory factory);
static bool checkColorOfConses(SharedConsFactory factory, Color color, const std::vector<Cons*> conses);


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

SCENARIO("cons allocation", "[ConsFactory]")
{
  GIVEN("A cons factory")
  {
    auto factory = makeFactory(8);
    WHEN("there is no cons allocated")
    {
      THEN("there is no cons for any color")
      {
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
    
    WHEN("one cons with no children is allocated")
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
        REQUIRE(getNumConses(factory) == 8u);
        REQUIRE(getNumConses(factory, factory->getFromRootColor()) == 1u);
        REQUIRE(getNumConses(factory, Color::Void) == 7u);
      }
    }

    WHEN("There as a cons with 2 children")
    {
      /*          
              o
             / \
            o   o 
      */
      Cons * pcons = factory->make(Object(factory->make(Lisp::nil,
                                                        Lisp::nil)),
                                   Object(factory->make(Lisp::nil,
                                                        Lisp::nil)));
      THEN("The new cons is root and has from-root color")
      {
        REQUIRE(pcons->getColor() == factory->getFromRootColor());
      }
      THEN("the new cons has ref-count 1")
      {
        REQUIRE(pcons->getRefCount() == 1u);
      }
      THEN("there is 1 root with from-color, 2 with from-color and 5 void conses")
      {
        REQUIRE(getNumConses(factory) == 8u);
        REQUIRE(getNumConses(factory, factory->getFromRootColor()) == 1u);
        REQUIRE(getNumConses(factory, factory->getToColor()) == 2u);
        REQUIRE(getNumConses(factory, Color::Void) == 5u);
      }
    }
  }
}

///////////////////
using V = std::vector<std::size_t>;
using CV = std::vector<Color>;

#include "lisp_cons_graph.h"
#include "lisp_cons_graph_node.h"
#include "lisp_cons_graph_edge.h"
#include "lisp_object.h"
#include "lisp_cons.h"
#include "lisp_nil.h"

using Object = Lisp::Object;
using Nil = Lisp::Nil;
using ConsGraph = Lisp::ConsGraph;




class ConsFactoryFixture : public ConsFactory
{
public:
  static const std::size_t undef; //= std::numeric_limits<std::size_t>::max();
  static const std::size_t error; //= std::numeric_limits<std::size_t>::max() - 1;

  ConsFactoryFixture(std::size_t pageSize=12) : ConsFactory(pageSize, 0, 0)
  {
  }
  
  bool checkColorOfConses(Color color, const std::vector<Cons*> conses)
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

  std::size_t checkNumConses(Color color)
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

  bool checkChildrenOfRootAndToColorConses() const
  {
    auto conses = getConses(getToColor());
    auto root = getConses(getToRootColor());
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


  
  bool checkParents(const Lisp::Cons * cons,
                    const std::vector<const Lisp::Cons*> & parents)
  {
    ConsGraph graph(*this);
    auto node = graph.findNode(cons);
    if(node)
    {
      return node->getConsParents() ==
         std::unordered_set<const Cons*>(parents.begin(),
                                         parents.end());
    }
    else
    {
      return false;
    }
  }

  bool checkParents(const Cons * cons, const Cons * p1)
  {
    return checkParents(cons, std::vector<const Cons*>({p1}));
  }

  bool checkParents(const Cons * cons, const Cons * p1, const Cons * p2)
  {
    return checkParents(cons, std::vector<const Cons*>({p1, p2}));
  }

  bool checkParents(const Cons * cons,
                    const Cons * p1,
                    const Cons * p2,
                    const Cons * p3)
  {
    return checkParents(cons, std::vector<const Cons*>({p1, p2, p3}));
  }

  bool checkParents(const Cons * cons,
                    const Cons * p1,
                    const Cons * p2,
                    const Cons * p3,
                    const Cons * p4)
  {
    return checkParents(cons, std::vector<const Cons*>({p1, p2, p3, p4}));
  }

  std::size_t getWeight(const Cons * parent, const Cons * child) const
  {
    ConsGraph graph(*this);
    auto edge = graph.findEdge(parent, child);
    if(edge)
    {
      return edge->getWeight();
    }
    else
    {
      return undef;
    }
  }
};

const std::size_t ConsFactoryFixture::undef = std::numeric_limits<std::size_t>::max();
const std::size_t ConsFactoryFixture::error = std::numeric_limits<std::size_t>::max() - 1;


std::unordered_set<Cons*> setOfConses(const std::vector<Cons*> & conses)
{
  return std::unordered_set<Cons*>(conses.begin(), conses.end());
}

std::unordered_set<Cons*> setOfConses(Cons * c1 = nullptr,
                                      Cons * c2 = nullptr,
                                      Cons * c3 = nullptr,
                                      Cons * c4 = nullptr,
                                      Cons * c5 = nullptr,
                                      Cons * c6 = nullptr,
                                      Cons * c7 = nullptr,
                                      Cons * c8 = nullptr,
                                      Cons * c9 = nullptr,
                                      Cons * c10 = nullptr)
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

TEST_CASE("empty_cons_factory", "[ConsFactory]")
{
  // [x] done
  ConsFactoryFixture factory(12);
  REQUIRE(factory.checkConses() == V({ 0, 0, 0, 0, 0, 0, 0, 0}));
  REQUIRE(factory.getWeight(nullptr, nullptr) == ConsFactoryFixture::undef);
}

TEST_CASE("alloc_cons_nil_nil_is_root_with_ref_count_1", "[ConsFactory]")
{
  // [x] done
  using V = std::vector<std::size_t>;
  ConsFactoryFixture factory(8);
  Cons * ptr = factory.make(Lisp::nil, Lisp::nil);
  REQUIRE(ptr);
  REQUIRE(ptr->getColor() == factory.getFromRootColor());
  REQUIRE(ptr->getRefCount() == 1u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 0, 0, 0, 7, 0}));
  REQUIRE(factory.getWeight(nullptr, ptr) == 1u);
}

TEST_CASE("alloc_cons_cons_cons_has_two_black_conses", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  ConsFactoryFixture factory(8);
  /*          
              o
             / \
            o   o 
  */
  Cons * cons = factory.make(Object(factory.make(Lisp::nil,
                                                 Lisp::nil)),
                             Object(factory.make(Lisp::nil,
                                                 Lisp::nil)));
  REQUIRE(cons);
  REQUIRE(factory.checkParents(cons->getCar().as<Lisp::Cons>(), cons));
  REQUIRE(factory.checkParents(cons->getCdr().as<Lisp::Cons>(), cons));
  REQUIRE(cons->getColor() == factory.getFromRootColor());
  REQUIRE(cons->getRefCount() == 1u);
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
  REQUIRE(factory.getWeight(cons, cons) == ConsFactoryFixture::undef);
  REQUIRE(factory.getWeight(cons, cons->getCarCell().as<Lisp::Cons>()) == 1u);
  REQUIRE(factory.getWeight(cons, cons->getCdrCell().as<Lisp::Cons>()) == 1u);
}

TEST_CASE("alloc_cons_cons_cons_get_car_cdr", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
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
                                      
TEST_CASE("get_all_reachable_conses", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
  ConsFactoryFixture factory(8);
  Object cons3;
  Object cons1(factory.make(Object(factory.make(Lisp::nil,
                                                Lisp::nil)),
                            Object(factory.make(Lisp::nil,
                                                Lisp::nil))));
  REQUIRE(factory.checkConses() == V({1, 0, 0, 2, 0, 0, 5, 0}));
  REQUIRE(setOfConses(factory.getReachableConses()) ==
          setOfConses(cons1.as<Cons>(),
                      cons1.as<Cons>()->getCarCell().as<Cons>(),
                      cons1.as<Cons>()->getCdrCell().as<Cons>()));
  {
    Object cons2(factory.make(Object(factory.make(Lisp::nil,
                                                  Lisp::nil)),
                              Object(factory.make(Lisp::nil,
                                                  Lisp::nil))));
    REQUIRE(factory.checkConses() == V({2, 0, 0, 4, 0, 0, 2, 0}));
    REQUIRE(setOfConses(factory.getReachableConses()) ==
            setOfConses(cons1.as<Cons>(),
                        cons1.as<Cons>()->getCarCell().as<Cons>(),
                        cons1.as<Cons>()->getCdrCell().as<Cons>(),
                        cons2.as<Cons>(),
                        cons2.as<Cons>()->getCarCell().as<Cons>(),
                        cons2.as<Cons>()->getCdrCell().as<Cons>()));
    cons3 = Object(factory.make(cons2.as<Cons>()->getCar(), Lisp::nil));
    REQUIRE(factory.checkConses() == V({3, 0, 0, 4, 0, 0, 1, 0}));
    REQUIRE(setOfConses(factory.getReachableConses()) ==
            setOfConses(cons1.as<Cons>(),
                        cons1.as<Cons>()->getCarCell().as<Cons>(),
                        cons1.as<Cons>()->getCdrCell().as<Cons>(),
                        cons2.as<Cons>(),
                        cons2.as<Cons>()->getCarCell().as<Cons>(),
                        cons2.as<Cons>()->getCdrCell().as<Cons>(),
                        cons3.as<Cons>()));
    /*
         o        o    o
        / \      / \  / \
       o   o    o   o  
    */
    auto c1 = cons1.as<Cons>();
    auto c2 = cons2.as<Cons>();
    auto c3 = cons3.as<Cons>();
    auto undef = ConsFactoryFixture::undef;
    REQUIRE(c3->getCarCell().as<Cons>() == c2->getCarCell().as<Cons>());
    REQUIRE(factory.checkParents(c3->getCarCell().as<Cons>(),
                                 c3,
                                 c2));
    REQUIRE(factory.checkParents(c2->getCarCell().as<Cons>(),
                                 c2,
                                 c3));

    REQUIRE(factory.getWeight(c1, c1->getCarCell().as<Cons>()) == 1u);
    REQUIRE(factory.getWeight(c1, c1->getCdrCell().as<Cons>()) == 1u);
    REQUIRE(factory.getWeight(c2, c2->getCarCell().as<Cons>()) == 0u);
    REQUIRE(factory.getWeight(c2, c2->getCdrCell().as<Cons>()) == 1u);
    REQUIRE(factory.getWeight(c3, c3->getCarCell().as<Cons>()) == 0u);
    REQUIRE(factory.getWeight(c3, c3->getCdrCell().as<Cons>()) == undef);
    

  }
  REQUIRE(factory.checkConses() == V({2, 0, 0, 5, 0, 0, 1, 0}));
  REQUIRE(setOfConses(factory.getReachableConses()) ==
          setOfConses(cons1.as<Cons>(),
                      cons1.as<Cons>()->getCarCell().as<Cons>(),
                      cons1.as<Cons>()->getCdrCell().as<Cons>(),
                      cons3.as<Cons>(),
                      cons3.as<Cons>()->getCarCell().as<Cons>()));
}

TEST_CASE("recycle_all_unreachable_conses", "[ConsFactory]")
{
  // Todo check reachable
  using V = std::vector<std::size_t>;
  ConsFactoryFixture factory(8);
  Object cons3;
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
    cons3 = Object(factory.make(cons2.as<Cons>()->getCar(), Lisp::nil));
    REQUIRE(factory.checkConses() == V({3, 0, 0, 4, 0, 0, 1, 0}));

    REQUIRE(factory.checkParents(cons2.as<Cons>()->getCar().as<Cons>(),
                                 cons3.as<Cons>(),
                                 cons2.as<Cons>()));


  }
  REQUIRE(factory.checkConses() == V({2, 0, 0, 5, 0, 0, 1, 0}));
  REQUIRE(setOfConses(factory.getReachableConses()) ==
          setOfConses(cons1.as<Cons>(),
                      cons1.as<Cons>()->getCarCell().as<Cons>(),
                      cons1.as<Cons>()->getCdrCell().as<Cons>(),
                      cons3.as<Cons>(),
                      cons3.as<Cons>()->getCarCell().as<Cons>()));
  factory.cycleGarbageCollector();
  REQUIRE(factory.checkConses() == V({2, 0, 0, 3, 0, 0, 3, 0}));
  REQUIRE(setOfConses(factory.getReachableConses()) ==
          setOfConses(factory.getConses(Color::White, Color::Free)));
}

TEST_CASE("object_copy_constructor_cons", "[ConsFactory]")
{
  using V = std::vector<std::size_t>;
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

