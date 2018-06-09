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

#include <lpp/core/lisp_cons_factory.h>
#include <lpp/core/types/cons.h>
// todo: replace ConsContainer
#include <lpp/core/types/lisp_cons_container.h>
#include <lpp/core/types/lisp_array.h>
#include <lpp/core/lisp_object.h>

#include <lpp/core/gc/collectible_graph.h>
#include <lpp/core/gc/collectible_edge.h>
#include <lpp/core/gc/collectible_node.h>

// helper types
using ConsFactory = Lisp::ConsFactory;
using SharedConsFactory = std::shared_ptr<ConsFactory>;
using Cons = Lisp::Cons;
using Cell = Lisp::Cell;
using Collectible = Lisp::Collectible;
using Color = ConsFactory::Color;
using Object = Lisp::Object;
using Nil = Lisp::Nil;
using CollectibleGraph = Lisp::CollectibleGraph;
using Array = Lisp::Array;

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

static std::unordered_set<Cell> Set();
static std::unordered_set<Cell> Set(const Cell & c);
template<typename... ARGS>
static std::unordered_set<Cell> Set(const Cell & c, ARGS... rest);
static std::unordered_set<Cell> Set(const std::vector<Cell> & cells);

static std::unordered_set<Cell> Parents(const CollectibleGraph & graph,
                                        const Cell & cell);


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
    CollectibleGraph graph(*factory);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(cons);
      REQUIRE(cons->isRoot());
      REQUIRE(cons->getColor() == factory->getFromRootColor());
      REQUIRE(cons->getRefCount() == 1u);
    }
    THEN("it has no parents")
    {
      REQUIRE(Parents(graph, *obj) == Set());
    }
    THEN("it is reachable")
    {
      REQUIRE(Set(factory->getReachable()) == Set(Cell(cons)));
    }
    THEN("there is 1 root cons with from-color and 7 void conses")
    {
      REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor() == 1u,
                                         Color::Void == 7u}));
    }
    WHEN("the cons is unrooted")
    {
      obj.reset();
      CollectibleGraph graph(*factory);
      THEN("it is a leaf cons with from-color, ref-count 0")
      {
        REQUIRE_FALSE(cons->isRoot());
        REQUIRE(cons->getColor() == factory->getFromColor());
      }
      THEN("there is 1 leaf conses with from-color and 7 void conses")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromColor() == 1u,
                                           Color::Void == 7u}));
      }
      THEN("there is no reachable cons")
      {
        REQUIRE(Set(factory->getReachable()) == Set());
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
      CollectibleGraph graph(*factory);
      THEN("its car and cdr is itself")
      {
        REQUIRE(cons->getCarCell().isA<Cons>());
        REQUIRE(cons->getCdrCell().isA<Cons>());
        REQUIRE(cons->getCarCell().as<Cons>() == cons);
        REQUIRE(cons->getCdrCell().as<Cons>() == cons);
      }
      THEN("it is its own parent")
      {
        REQUIRE(Parents(graph, *obj) == Set(Cell(cons)));
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
    CollectibleGraph graph(*factory);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(cons);
      REQUIRE(cons->isRoot());
      REQUIRE(cons->getColor() == factory->getFromRootColor());
      REQUIRE(cons->getRefCount() == 0u);
    }
    THEN("the parent of its children is the cons")
    {
      REQUIRE(Parents(graph, cons) == Set());
      REQUIRE(Parents(graph, cons->getCarCell()) == Set(Cell(cons)));
      REQUIRE(Parents(graph, cons->getCdrCell()) == Set(Cell(cons)));
    }
    THEN("the cons and its children are reachable")
    {
      REQUIRE(Set(factory->getReachable()) == Set(Cell(cons),
                                                  cons->getCarCell(),
                                                  cons->getCdrCell()));
    }
    THEN("there is 1 root cons with from-color and 7 void conses")
    {
      REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor() == 1u,
                                         factory->getFromColor() == 2u,
                                         Color::Void == 5u}));
    }
    WHEN("There are references to its children")
    {
      {
        auto car = cons->getCar();
        auto cdr = cons->getCdr();
        CollectibleGraph graph(*factory);
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
    CollectibleGraph graph(*factory);
    THEN("there are 2 conses with from-root-color and 4 conses with from-color")
    {
      REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u,
                                         factory->getFromColor() == 4u,
                                         Color::Void == 2u}));
    }
    THEN("there are 6 reachable conses")
    {
      REQUIRE(Set(factory->getReachable()) == Set(cons1,
                                                 cons1.as<Cons>()->getCarCell(),
                                                 cons1.as<Cons>()->getCdrCell(),
                                                 cons2,
                                                 cons2.as<Cons>()->getCarCell(),
                                                 cons2.as<Cons>()->getCdrCell()));
    }
    WHEN("there are 2 conses with 4 children")
    {
      /*
           o        o    o
          / \      / \  / \
         o   o    o   o
      */

      Object cons3(factory->make(cons2.as<Cons>()->getCar(), Lisp::nil));
      CollectibleGraph graph(*factory);
      THEN("there are 2 conses with from-root-color and 4 conses with from-color")
      {
        REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==3u,
                                           factory->getFromColor() == 4u,
                                           Color::Void == 1u}));
      }
      THEN("all children of cons1, cons2 and cons3 are reachable")
      {
        REQUIRE(Set(factory->getReachable()) == Set(cons1,
                                                    cons1.as<Cons>()->getCarCell(),
                                                    cons1.as<Cons>()->getCdrCell(),
                                                    cons2,
                                                    cons2.as<Cons>()->getCarCell(),
                                                    cons2.as<Cons>()->getCdrCell(),
                                                    cons3,
                                                    cons3.as<Cons>()->getCarCell()));
      }
      WHEN("cons2 is unset")
      {
        /*
                o            o
               / \          / \
              o   o        o
        */
        cons2 = Lisp::nil;
        CollectibleGraph graph(*factory);
        THEN("there are 2 root conses and 3 leafes")
        {
          REQUIRE(checkConses(factory, 8u, { factory->getFromRootColor()==2u,
                                             factory->getFromColor() == 5u,
                                             Color::Void == 1u}));
          REQUIRE(Set(factory->getReachable()) == Set(cons1,
                                                      cons1.as<Cons>()->getCarCell(),
                                                      cons1.as<Cons>()->getCdrCell(),
                                                      cons3.as<Cons>(),
                                                      cons3.as<Cons>()->getCarCell()));
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
          REQUIRE(checkConses(factory,
                              8u,
                              { factory->getFromRootColor()==1u,
                                factory->getToColor() == 2u,
                                Color::Void == 5u}));
        }
      }
    }
  }
}

#if 0
SCENARIO("one array without elements", "[ConsFactory]")
{
  GIVEN("A root array")
  {
    auto factory = makeFactory(8);
    auto obj = std::make_shared<Object>(factory->makeArray());
    auto array = obj->as<Array>();
    CollectibleGraph graph(*factory);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(array);
      REQUIRE(array->isRoot());
      REQUIRE(array->getColor() == factory->getFromRootColor());
      REQUIRE(array->getRefCount() == 1u);
    }
    THEN("it has no parents")
    {
      arrayHasParents(graph, array);
    }
    THEN("it is reachable")
    {
      REQUIRE(factory->getReachableArraysAsSet() == setOfArrays(array));
    }
    WHEN("the cons is unrooted")
    {
      obj.reset();
      CollectibleGraph graph(*factory);
      THEN("it is a leaf cons with from-color, ref-count 0")
      {
        REQUIRE_FALSE(array->isRoot());
        REQUIRE(array->getColor() == factory->getFromColor());
      }
      THEN("there is no reachable array")
      {
        REQUIRE(factory->getReachableArraysAsSet() == setOfArrays());
      }
    }
  }
}
#endif

SCENARIO("recycle ConsContainer", "[ConsFactory]")
{
  auto factory = std::make_shared<ConsFactory>(8, 1);
  auto container = factory->makeContainer();
  GIVEN("a container with 3 conses")
  {
    Color color = factory->getFromRootColor();
    REQUIRE(container->getColor() == color);
    factory->stepGarbageCollector();
    factory->stepGarbageCollector();
    REQUIRE(factory->getFromRootColor() !=  color);
    REQUIRE(container->getColor() == factory->getFromRootColor());
    factory->disableGarbageCollector();
    container->pushCons(factory->make(Lisp::nil, Lisp::nil));
    container->pushCons(factory->make(Lisp::nil, Lisp::nil));
    container->pushCons(factory->make(Lisp::nil, Lisp::nil));
    REQUIRE((*container)[0]->getColor() == factory->getFromRootColor());
    REQUIRE((*container)[1]->getColor() == factory->getFromRootColor());
    REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
    REQUIRE((*container)[0]->getIndex() == 0);
    REQUIRE((*container)[1]->getIndex() == 1);
    REQUIRE((*container)[2]->getIndex() == 2);
    WHEN("all conses have been marked")
    {
      factory->enableGarbageCollector();
      REQUIRE((*container)[0]->getColor() == factory->getFromRootColor());
      REQUIRE((*container)[1]->getColor() == factory->getFromRootColor());
      REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
      REQUIRE(container->getGcTop() == 0);
      factory->stepGarbageCollector();
      REQUIRE((*container)[0]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[1]->getColor() == factory->getFromRootColor());
      REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
      REQUIRE(container->getGcTop() == 1);
      factory->stepGarbageCollector();
      REQUIRE((*container)[0]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[1]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
      REQUIRE(container->getGcTop() == 2);
      factory->stepGarbageCollector();
      REQUIRE((*container)[0]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[1]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[2]->getColor() == factory->getToRootColor());
      REQUIRE(container->getGcTop() == 3);
      color = factory->getFromRootColor();
      THEN("container is marked")
      {
	REQUIRE(container->getColor() == color);
	factory->stepGarbageCollector(); // mark container
	REQUIRE(container->getColor() != color);
	REQUIRE(container->getGcTop() == 0);
	factory->stepGarbageCollector(); // swap colors
	REQUIRE(container->getGcTop() == 0);
	REQUIRE(factory->getToRootColor() == color);
	REQUIRE((*container)[0]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
      }
      THEN("adding conses to marked container")
      {
	REQUIRE(container->getColor() == color);
	factory->stepGarbageCollector(); // mark container
	REQUIRE(container->getColor() != color);
	REQUIRE(container->getColor() == factory->getToRootColor());
	factory->disableGarbageCollector();
	container->pushCons(factory->make(Lisp::nil, Lisp::nil));
	REQUIRE((*container)[0]->getColor() == factory->getToRootColor());
	REQUIRE((*container)[1]->getColor() == factory->getToRootColor());
	REQUIRE((*container)[2]->getColor() == factory->getToRootColor());
	REQUIRE((*container)[3]->getColor() == factory->getToRootColor());
	REQUIRE(container->getGcTop() == 0);
	factory->enableGarbageCollector();
	factory->stepGarbageCollector(); // swap colors
	REQUIRE(container->getGcTop() == 0);
	REQUIRE(factory->getToRootColor() == color);
	REQUIRE((*container)[0]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[3]->getColor() == factory->getFromRootColor());
      }
    }
    WHEN("part of conses have been marked")
    {
      factory->enableGarbageCollector();
      factory->stepGarbageCollector();
      factory->stepGarbageCollector();
      REQUIRE((*container)[0]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[1]->getColor() == factory->getToRootColor());
      REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
      REQUIRE(container->getGcTop() == 2);
      REQUIRE(container->getColor() == factory->getFromRootColor());
      THEN("adding cons at the end has from color")
      {
	factory->disableGarbageCollector();
	container->pushCons(factory->make(Lisp::nil, Lisp::nil));
	REQUIRE((*container)[3]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == factory->getToRootColor());
	container->setCons(1, factory->make(Lisp::nil, Lisp::nil));
	container->setCons(3, factory->make(Lisp::nil, Lisp::nil));
	REQUIRE((*container)[0]->getColor() == factory->getToRootColor());
	REQUIRE((*container)[1]->getColor() == factory->getToRootColor());
	REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[3]->getColor() == factory->getToRootColor());
        factory->enableGarbageCollector();
        factory->stepGarbageCollector();
        REQUIRE(container->getGcTop() == 3);
        REQUIRE((*container)[2]->getColor() == factory->getToRootColor());
        REQUIRE(container->getColor() == factory->getFromRootColor());
        color = container->getColor();
        factory->stepGarbageCollector();
        REQUIRE(container->getGcTop() == 4);
        REQUIRE(container->getColor() == color);
        factory->stepGarbageCollector();
        REQUIRE(container->getColor() != color);
        REQUIRE(container->getColor() == factory->getToRootColor());
        factory->stepGarbageCollector();
        REQUIRE(container->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[0]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[2]->getColor() == factory->getFromRootColor());
	REQUIRE((*container)[3]->getColor() == factory->getFromRootColor());
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
  auto nConses = factory->numConses(color);
  if(color != Color::Void)
  {
    auto conses = factory->getConses(color);
    bool colorOfConsesEqual = factory->checkSanity(color);
    CHECK(colorOfConsesEqual);
    CHECK(nConses == conses.size());
    return (nConses == conses.size() && colorOfConsesEqual) ? nConses : error;
  }
  else
  {
    return nConses;
  }
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

std::unordered_set<Cell> Set()
{
  return std::unordered_set<Cell>();
}

std::unordered_set<Cell> Set(const Cell & c)
{
  return std::unordered_set<Cell>({c});
}

template<typename... ARGS>
std::unordered_set<Cell> Set(const Cell & c, ARGS... rest)
{
  auto tmp = Set(rest...);
  tmp.insert(c);
  return tmp;
}

std::unordered_set<Cell> Set(const std::vector<Cell> & cells)
{
  return std::unordered_set<Cell>(cells.begin(), cells.end());
}

// todo: replace cons* with cell
static std::unordered_set<Cell> Parents(const CollectibleGraph & graph,
                                        const Cell & cell)
{
  auto node = graph.findNode(cell);
  if(node)
  {
    return node->getParents();
  }
  else
  {
    return Set();
  }
}
