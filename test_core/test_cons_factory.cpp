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

#include "core/lisp_cons_factory.h"
#include "core/types/lisp_cons.h"
#include "core/lisp_object.h"
#include "core/types/lisp_nil.h"
#include "simul/lisp_cons_graph.h"
#include "simul/lisp_cons_graph_edge.h"
#include "simul/lisp_cons_graph_node.h"

// helper types
using ConsFactory = Lisp::ConsFactory;
using SharedConsFactory = std::shared_ptr<ConsFactory>;
using Cons = Lisp::Cons;
using Color = ConsFactory::Color;
using Object = Lisp::Object;
using Nil = Lisp::Nil;
using ConsGraph = Lisp::ConsGraph;

// helper constants
static const std::size_t undef = std::numeric_limits<std::size_t>::max();
static const std::size_t error = std::numeric_limits<std::size_t>::max() - 1;

// helper functions
static SharedConsFactory makeFactory(std::size_t pageSize=12);
static std::size_t getNumConses(SharedConsFactory factory, Color color);
static std::size_t getNumConses(SharedConsFactory factory);
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
static std::size_t getWeight(const ConsGraph & graph,
                             const Cons * parent,
                             const Cons * child);
static bool consHasParents(const ConsGraph & graph,
                           Cons * cons,
                           const std::vector<Cons*> & parents);
static bool consHasParents(const ConsGraph & graph,
                           Cons * cons,
                           Cons * c1 = nullptr,
                           Cons * c2 = nullptr,
                           Cons * c3 = nullptr,
                           Cons * c4 = nullptr,
                           Cons * p5 = nullptr,
                           Cons * p6 = nullptr,
                           Cons * p7 = nullptr,
                           Cons * p8 = nullptr,
                           Cons * p9 = nullptr,
                           Cons * p10 = nullptr);

SCENARIO("no cons allocated", "[ConsFactory]")
{
  GIVEN("An empty cons factory")
  {
    auto factory = makeFactory(8);
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
    THEN("the from color is white")
    {
      REQUIRE(factory->getFromColor() == Color::White);
      REQUIRE(factory->getFromRootColor() == Color::WhiteRoot);
    }
  }
}

SCENARIO("one cons without children", "[ConsFactory]")
{
  GIVEN("A root cons")
  {
    /*
         +
        / \
    */
    auto factory = makeFactory(8);
    auto obj = std::make_shared<Object>(factory->make(Lisp::nil, Lisp::nil));
    auto cons = obj->as<Cons>();
    ConsGraph graph(*factory);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(cons);
      REQUIRE(cons->isRoot());
      REQUIRE(cons->getColor() == factory->getFromRootColor());
      REQUIRE(cons->getRefCount() == 1u);
    }
    THEN("its weight from root is 1")
    {
      REQUIRE(getWeight(graph, nullptr, cons) == 1u);
    }
    THEN("it has no parents")
    {
      consHasParents(graph, cons);
    }
    THEN("it is reachable")
    {
      REQUIRE(factory->getReachableConsesAsSet() == setOfConses(cons));
    }
    THEN("there is 1 root cons with from-color and 7 void conses")
    {
      REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor() == 1u,
                                         Color::Void == 7u}));
    }
    WHEN("the cons is unrooted")
    {
      obj.reset();
      ConsGraph graph(*factory);
      THEN("it is a leaf cons with from-color, ref-count 0")
      {
        REQUIRE_FALSE(cons->isRoot());
        REQUIRE(cons->getColor() == factory->getFromColor());
      }
      THEN("there is no edge from root to cons")
      {
        REQUIRE(getWeight(graph, nullptr, cons) == undef);
      }
      THEN("there is no self-ref")
      {
        REQUIRE(getWeight(graph, cons, cons) == undef);
      }
      THEN("there is 1 leaf conses with from-color and 7 void conses")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromColor() == 1u,
                                           Color::Void == 7u}));
      }
      THEN("there is no reachable cons")
      {
        REQUIRE(factory->getReachableConsesAsSet() == setOfConses());
      }
    }
  }
}

SCENARIO("one cons with self-ref", "[ConsFactory]")
{
  GIVEN("A root cons")
  {
    /*
         +
        / \
    */
    WHEN("the cons' children is set to it self")
    {
      auto factory = makeFactory(8);
      auto obj = std::make_shared<Object>(factory->make(Lisp::nil, Lisp::nil));
      auto cons = obj->as<Cons>();
      cons->setCar(*obj);
      cons->setCdr(*obj);
      ConsGraph graph(*factory);
      THEN("its car and cdr is itself")
      {
        REQUIRE(cons->getCarCell().isA<Cons>());
        REQUIRE(cons->getCdrCell().isA<Cons>());
        REQUIRE(cons->getCarCell().as<Cons>() == cons);
        REQUIRE(cons->getCdrCell().as<Cons>() == cons);
      }
      THEN("it is its own parent")
      {
        REQUIRE(consHasParents(graph, cons, cons));
      }
      THEN("its weight from root is 1")
      {
        REQUIRE(getWeight(graph, nullptr, cons) == 1u);
      }
      THEN("the weight from cons to car and cdr is 0")
      {
        REQUIRE(getWeight(graph, cons, cons->getCarCell().as<Cons>()) == 0u);
        REQUIRE(getWeight(graph, cons, cons->getCdrCell().as<Cons>()) == 0u);
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
  GIVEN("A cons factory with 2 children")
  {
    auto factory = makeFactory(8);
    auto cons = factory->make(Object(factory->make(Lisp::nil, Lisp::nil)),
                              Object(factory->make(Lisp::nil, Lisp::nil)));
    ConsGraph graph(*factory);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(cons);
      REQUIRE(cons->isRoot());
      REQUIRE(cons->getColor() == factory->getFromRootColor());
      REQUIRE(cons->getRefCount() == 0u);
    }
    THEN("the parent of its children is the cons")
    {
      REQUIRE(consHasParents(graph, cons));
      REQUIRE(consHasParents(graph, cons->getCarCell().as<Cons>(), cons));
      REQUIRE(consHasParents(graph, cons->getCdrCell().as<Cons>(), cons));
    }
    THEN("the cons and its children are reachable")
    {
      REQUIRE(factory->getReachableConsesAsSet() == setOfConses(cons,
                                                                cons->getCarCell().as<Cons>(),
                                                                cons->getCdrCell().as<Cons>()));
    }
    THEN("there is 1 root cons with from-color and 7 void conses")
    {
      REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor() == 1u,
                                         factory->getFromColor() == 2u,
                                         Color::Void == 5u}));
    }
    THEN("the edge weight from cons to cons is undef")
    {
      REQUIRE(getWeight(graph, cons, cons) == undef);
    }
    THEN("the edge weight from root to cons is 3")
    {
      REQUIRE(getWeight(graph, nullptr, cons) == 3u);
    }
    THEN("the edge weight from cons to children is 1")
    {
      REQUIRE(getWeight(graph, cons, cons->getCarCell().as<Cons>()) == 1u);
      REQUIRE(getWeight(graph, cons, cons->getCdrCell().as<Cons>()) == 1u);
    }
    WHEN("There are references to its children")
    {
      {
        auto car = cons->getCar();
        auto cdr = cons->getCdr();
        ConsGraph graph(*factory);
        THEN("reference count is 1")
        {
          REQUIRE(car.as<Cons>()->getRefCount() == 1u);
          REQUIRE(cdr.as<Cons>()->getRefCount() == 1u);
        }
        THEN("there are 3 conses with root-from-color and 5 void conses")
        {
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==3u,
                                             Color::Void == 5u}));
        }
        THEN("the edge weight from root to cons is 0")
        {
          REQUIRE(getWeight(graph, nullptr, cons) == 0u);
        }
        THEN("the edge weight from cons to children is 0")
        {
          REQUIRE(getWeight(graph, cons, cons->getCarCell().as<Cons>()) == 0u);
          REQUIRE(getWeight(graph, cons, cons->getCdrCell().as<Cons>()) == 0u);
        }
      }
      THEN("finally there is cons with root-from-clor and 2 conses with to-color")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u,
                                           factory->getFromColor() == 2u,
                                           Color::Void == 5u}));
      }
    }
  }
}

SCENARIO("3 conses with 4 children", "[ConsFactory]")
{
  GIVEN("2 conses")
  {
    /*
           o        o 
          / \      / \
         o   o    o   o
      */

    auto factory = makeFactory(8);
    Object cons1(factory->make(Object(factory->make(Lisp::nil,
                                                    Lisp::nil)),
                               Object(factory->make(Lisp::nil,
                                                    Lisp::nil))));
    Object cons2(factory->make(Object(factory->make(Lisp::nil,
                                                    Lisp::nil)),
                               Object(factory->make(Lisp::nil,
                                                    Lisp::nil))));
    ConsGraph graph(*factory);
    THEN("there are 2 conses with from-root-color and 4 conses with from-color")
    {
      REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u,
                                         factory->getFromColor() == 4u,
                                         Color::Void == 2u}));
    }
    THEN("there are 6 reachable conses")
    {
      REQUIRE(factory->getReachableConsesAsSet() == setOfConses(cons1.as<Cons>(),
                                                                cons1.as<Cons>()->getCarCell().as<Cons>(),
                                                                cons1.as<Cons>()->getCdrCell().as<Cons>(),
                                                                cons2.as<Cons>(),
                                                                cons2.as<Cons>()->getCarCell().as<Cons>(),
                                                                cons2.as<Cons>()->getCdrCell().as<Cons>()));
    }
    THEN("weights to root conses are 3")
    {
      REQUIRE(getWeight(graph, nullptr, cons1.as<Cons>()) == 3u);
      REQUIRE(getWeight(graph, nullptr, cons2.as<Cons>()) == 3u);
    }
    THEN("weights to leaf conses are 1")
    {
      REQUIRE(getWeight(graph, cons1.as<Cons>(), cons1.as<Cons>()->getCarCell().as<Cons>()) == 1u);
      REQUIRE(getWeight(graph, cons1.as<Cons>(), cons1.as<Cons>()->getCdrCell().as<Cons>()) == 1u);
      REQUIRE(getWeight(graph, cons2.as<Cons>(), cons2.as<Cons>()->getCarCell().as<Cons>()) == 1u);
      REQUIRE(getWeight(graph, cons2.as<Cons>(), cons2.as<Cons>()->getCdrCell().as<Cons>()) == 1u);
    }
    WHEN("there are 2 conses with 4 children")
    {
      /*
           o        o    o
          / \      / \  / \
         o   o    o   o
      */

      Object cons3(factory->make(cons2.as<Cons>()->getCar(), Lisp::nil));
      ConsGraph graph(*factory);
      THEN("there are 2 conses with from-root-color and 4 conses with from-color")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==3u,
                                           factory->getFromColor() == 4u,
                                           Color::Void == 1u}));
      }
      THEN("all children of cons1, cons2 and cons3 are reachable")
      {
        REQUIRE(factory->getReachableConsesAsSet() == setOfConses(cons1.as<Cons>(),
                                                                  cons1.as<Cons>()->getCarCell().as<Cons>(),
                                                                  cons1.as<Cons>()->getCdrCell().as<Cons>(),
                                                                  cons2.as<Cons>(),
                                                                  cons2.as<Cons>()->getCarCell().as<Cons>(),
                                                                  cons2.as<Cons>()->getCdrCell().as<Cons>(),
                                                                  cons3.as<Cons>(),
                                                                  cons3.as<Cons>()->getCarCell().as<Cons>()));
      }
      THEN("weight to cons1 are 3,2,1")
      {
        REQUIRE(getWeight(graph, nullptr, cons1.as<Cons>()) == 3u);
        REQUIRE(getWeight(graph, nullptr, cons2.as<Cons>()) == 2u);
        REQUIRE(getWeight(graph, nullptr, cons3.as<Cons>()) == 1u);
      }
      THEN("weights to leaf conses are 1 or 0")
      {
        REQUIRE(getWeight(graph, cons1.as<Cons>(), cons1.as<Cons>()->getCarCell().as<Cons>()) == 1u);
        REQUIRE(getWeight(graph, cons1.as<Cons>(), cons1.as<Cons>()->getCdrCell().as<Cons>()) == 1u);
        REQUIRE(getWeight(graph, cons2.as<Cons>(), cons2.as<Cons>()->getCdrCell().as<Cons>()) == 1u);
        REQUIRE(getWeight(graph, cons2.as<Cons>(), cons2.as<Cons>()->getCarCell().as<Cons>()) == 0u);
        REQUIRE(getWeight(graph, cons3.as<Cons>(), cons3.as<Cons>()->getCarCell().as<Cons>()) == 0u);
      }
      WHEN("cons2 is unset")
      {
        /*
                o            o
               / \          / \
              o   o        o
        */
        cons2 = Lisp::nil;
        ConsGraph graph(*factory);
        THEN("there are 2 root conses and 3 leafes")
        {
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u,
                                             factory->getFromColor() == 5u,
                                             Color::Void == 1u}));
          REQUIRE(factory->getReachableConsesAsSet() == setOfConses(cons1.as<Cons>(),
                                                                    cons1.as<Cons>()->getCarCell().as<Cons>(),
                                                                    cons1.as<Cons>()->getCdrCell().as<Cons>(),
                                                                    cons3.as<Cons>(),
                                                                    cons3.as<Cons>()->getCarCell().as<Cons>()));
        }
        THEN("weights to root conses are 3 and 2")
        {
          REQUIRE(getWeight(graph, nullptr, cons1.as<Cons>()) == 3u);
          REQUIRE(getWeight(graph, nullptr, cons3.as<Cons>()) == 2u);
        }
        THEN("weights to leaf conses are 1")
        {
          REQUIRE(getWeight(graph, cons1.as<Cons>(), cons1.as<Cons>()->getCarCell().as<Cons>()) == 1u);
          REQUIRE(getWeight(graph, cons1.as<Cons>(), cons1.as<Cons>()->getCdrCell().as<Cons>()) == 1u);
          REQUIRE(getWeight(graph, cons3.as<Cons>(), cons3.as<Cons>()->getCarCell().as<Cons>()) == 1u);
        }
        WHEN("cycle garbage collector")
        {
          REQUIRE(factory->getFromColor() == Color::White);
          factory->cycleGarbageCollector();
          THEN("there are 2 from-root-color and 3 to-color conses")
          {
            REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u,
                                               factory->getToColor() == 3u,
                                               Color::Void == 3u}));
          }
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
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getFromColor() == 2u, Color::Void == 5u}));
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
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getFromColor() == 2u, Color::Void == 5u}));
        }
        WHEN("both conses are unset")
        {
          cons1 = Lisp::nil;
          REQUIRE(cons1.isA<Nil>());
          THEN("there is no root and 3 leaf conses")
          {
            REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==0u, factory->getFromColor() == 3u, Color::Void == 5u}));
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
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getFromColor() == 5u, Color::Void == 2u}));
      }
      WHEN("cycle garbage collector")
      {
        factory->cycleGarbageCollector();
        THEN("there is no root and 3 leaf conses")
        {
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==1u, factory->getToColor() == 2u, Color::Void == 5u}));
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
  auto conses = factory->getConses(color);
  auto nConses = factory->numConses(color);
  bool colorOfConsesEqual = Lisp::checkColorOfConses(*factory, color);
  CHECK(colorOfConsesEqual);
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

static std::size_t getWeight(const ConsGraph & graph, const Cons * parent, const Cons * child)
{
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

static bool consHasParents(const ConsGraph & graph,
                           Cons * cons,
                           const std::vector<Cons*> & parents)
{
  auto node = graph.findNode(cons);
  if(node)
  {
    return node->getParents() == setOfConses(parents);
  }
  else
  {
    return false;
  }
}

static bool consHasParents(const ConsGraph & graph,
                           Cons * cons,
                           Cons * c1,
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
  auto node = graph.findNode(cons);
  if(node)
  {
    return node->getParents() == setOfConses(c1, c2, c3, c4, c5,
                                             c6, c7, c8, c9, c10);
  }
  else
  {
    return false;
  }
}
