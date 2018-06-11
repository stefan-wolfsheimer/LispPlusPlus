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

#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/cons.h>
// todo: replace ConsContainer
#include <lpp/core/types/lisp_cons_container.h>
#include <lpp/core/types/lisp_array.h>
#include <lpp/core/lisp_object.h>

#include <lpp/core/gc/collectible_graph.h>
#include <lpp/core/gc/collectible_edge.h>
#include <lpp/core/gc/collectible_node.h>

// helper types
using GarbageCollector = Lisp::GarbageCollector;
using Cons = Lisp::Cons;
using Cell = Lisp::Cell;
using Collectible = Lisp::Collectible;
using Color = Lisp::Color;
using Object = Lisp::Object;
using Nil = Lisp::Nil;
using CollectibleGraph = Lisp::CollectibleGraph;
using Array = Lisp::Array;

// helper constants
static const std::size_t undef = std::numeric_limits<std::size_t>::max();
static const std::size_t error = std::numeric_limits<std::size_t>::max() - 1;

// helper functions
static std::shared_ptr<GarbageCollector> makeCollector(std::size_t pageSize=12);
static std::size_t getNumCollectible(std::shared_ptr<GarbageCollector> factory,
                                     Color color);
static std::size_t getNumCollectible(std::shared_ptr<GarbageCollector> factory);
static std::pair<Color, std::size_t> operator==(Color, std::size_t);
static bool checkConses(std::shared_ptr<GarbageCollector> factory,
                        const std::vector<std::pair<Color, std::size_t> > & conses);
static bool checkConses(std::shared_ptr<GarbageCollector> factory,
                        std::size_t total,
                        const std::vector<std::pair<Color, std::size_t> > & conses);

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
    auto coll = makeCollector(8);
    THEN("there is no cons for any color")
    {
      REQUIRE(getNumCollectible(coll, Color::Void) == 0u);
      REQUIRE(getNumCollectible(coll, Color::White) == 0u);
      REQUIRE(getNumCollectible(coll, Color::Grey) == 0u);
      REQUIRE(getNumCollectible(coll, Color::Black) == 0u);
      REQUIRE(getNumCollectible(coll, Color::WhiteRoot) == 0u);
      REQUIRE(getNumCollectible(coll, Color::GreyRoot) == 0u);
      REQUIRE(getNumCollectible(coll, Color::BlackRoot) == 0u);
      REQUIRE(getNumCollectible(coll, Color::Free) == 0u);
    }
    THEN("the from color is white")
    {
      REQUIRE(coll->getFromColor() == Color::White);
      REQUIRE(coll->getFromRootColor() == Color::WhiteRoot);
    }
  }
}

SCENARIO("one cons without children", "[ConsColl]")
{
  GIVEN("A root cons")
  {
    /*
         +
        / \
    */
    auto coll = makeCollector(8);
    auto obj = std::make_shared<Object>(coll->makeCons(Lisp::nil, Lisp::nil));
    auto cons = obj->as<Cons>();
    CollectibleGraph graph(*coll);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(cons);
      REQUIRE(cons->isRoot());
      REQUIRE(cons->getColor() == coll->getFromRootColor());
      REQUIRE(cons->getRefCount() == 1u);
    }
    THEN("it has no parents")
    {
      REQUIRE(Parents(graph, *obj) == Set());
    }
    THEN("it is reachable")
    {
      REQUIRE(Set(coll->getReachable()) == Set(Cell(cons)));
    }
    THEN("there is 1 root cons with from-color and 7 void conses")
    {
      REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor() == 1u,
                                         Color::Void == 7u}));
    }
    WHEN("the cons is unrooted")
    {
      obj.reset();
      CollectibleGraph graph(*coll);
      THEN("it is a leaf cons with from-color, ref-count 0")
      {
        REQUIRE_FALSE(cons->isRoot());
        REQUIRE(cons->getColor() == coll->getFromColor());
      }
      THEN("there is 1 leaf conses with from-color and 7 void conses")
      {
        REQUIRE(checkConses(coll, 8u, { coll->getFromColor() == 1u,
                                           Color::Void == 7u}));
      }
      THEN("there is no reachable cons")
      {
        REQUIRE(Set(coll->getReachable()) == Set());
      }
    }
  }
}

SCENARIO("one cons with self-ref", "[ConsColl]")
{
  GIVEN("A root cons")
  {
    /*
         +
        / \
    */
    WHEN("the cons' children is set to it self")
    {
      auto coll = makeCollector(8);
      auto obj = std::make_shared<Object>(coll->makeCons(Lisp::nil, Lisp::nil));
      auto cons = obj->as<Cons>();
      cons->setCar(*obj);
      cons->setCdr(*obj);
      CollectibleGraph graph(*coll);
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

SCENARIO("a cons with 2 children", "[ConsColl]")
{
  /*
              o
             / \
            o   o 
  */
  GIVEN("A cons coll with 2 children")
  {
    auto coll = makeCollector(8);
    auto cons = coll->makeCons(Object(coll->makeCons(Lisp::nil, Lisp::nil)),
                                  Object(coll->makeCons(Lisp::nil, Lisp::nil)));
    CollectibleGraph graph(*coll);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(cons);
      REQUIRE(cons->isRoot());
      REQUIRE(cons->getColor() == coll->getFromRootColor());
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
      REQUIRE(Set(coll->getReachable()) == Set(Cell(cons),
                                                  cons->getCarCell(),
                                                  cons->getCdrCell()));
    }
    THEN("there is 1 root cons with from-color and 7 void conses")
    {
      REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor() == 1u,
                                         coll->getFromColor() == 2u,
                                         Color::Void == 5u}));
    }
    WHEN("There are references to its children")
    {
      {
        auto car = cons->getCar();
        auto cdr = cons->getCdr();
        CollectibleGraph graph(*coll);
        THEN("reference count is 1")
        {
          REQUIRE(car.as<Cons>()->getRefCount() == 1u);
          REQUIRE(cdr.as<Cons>()->getRefCount() == 1u);
        }
        THEN("there are 3 conses with root-from-color and 5 void conses")
        {
          REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==3u,
                                             Color::Void == 5u}));
        }
      }
      THEN("finally there is cons with root-from-clor and 2 conses with to-color")
      {
        REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==1u,
                                           coll->getFromColor() == 2u,
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

    auto coll = makeCollector(8);
    Object cons1(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil)),
                                   Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil))));
    Object cons2(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil)),
                               Object(coll->makeCons(Lisp::nil,
                                                        Lisp::nil))));
    CollectibleGraph graph(*coll);
    THEN("there are 2 conses with from-root-color and 4 conses with from-color")
    {
      REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==2u,
                                         coll->getFromColor() == 4u,
                                         Color::Void == 2u}));
    }
    THEN("there are 6 reachable conses")
    {
      REQUIRE(Set(coll->getReachable()) == Set(cons1,
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

      Object cons3(coll->makeCons(cons2.as<Cons>()->getCar(), Lisp::nil));
      CollectibleGraph graph(*coll);
      THEN("there are 2 conses with from-root-color and 4 conses with from-color")
      {
        REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==3u,
                                           coll->getFromColor() == 4u,
                                           Color::Void == 1u}));
      }
      THEN("all children of cons1, cons2 and cons3 are reachable")
      {
        REQUIRE(Set(coll->getReachable()) == Set(cons1,
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
        CollectibleGraph graph(*coll);
        THEN("there are 2 root conses and 3 leafes")
        {
          REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==2u,
                                             coll->getFromColor() == 5u,
                                             Color::Void == 1u}));
          REQUIRE(Set(coll->getReachable()) == Set(cons1,
                                                      cons1.as<Cons>()->getCarCell(),
                                                      cons1.as<Cons>()->getCdrCell(),
                                                      cons3.as<Cons>(),
                                                      cons3.as<Cons>()->getCarCell()));
        }
        WHEN("cycle garbage collector")
        {
          REQUIRE(coll->getFromColor() == Color::White);
          coll->cycleCollector();
          THEN("there are 2 from-root-color and 3 to-color conses")
          {
            REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==2u,
                                               coll->getToColor() == 3u,
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
    auto coll = makeCollector(8);
    Object cons1(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil)),
                                   Object(coll->makeCons(Lisp::nil,
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
        REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==1u, coll->getFromColor() == 2u, Color::Void == 5u}));
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
          REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==1u, coll->getFromColor() == 2u, Color::Void == 5u}));
        }
        WHEN("both conses are unset")
        {
          cons1 = Lisp::nil;
          REQUIRE(cons1.isA<Nil>());
          THEN("there is no root and 3 leaf conses")
          {
            REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==0u, coll->getFromColor() == 3u, Color::Void == 5u}));
          }
          WHEN("cycle garbage location")
          {
            coll->cycleCollector();
            THEN("there is no root and 3 leaf conses")
            {
              REQUIRE(checkConses(coll, 8u, { Color::Void == 8u}));
            }
          }
        }
      }
    }
  }
}

SCENARIO("copy cons object with object assignement operator", "[ConsColl]")
{
  GIVEN("conses with 4 children")
  {
    auto coll = makeCollector(8);
    Object cons1(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil)),
                                   Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil))));
    Object cons2(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                            Lisp::nil)),
                                   Object(coll->makeCons(Lisp::nil,
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
        REQUIRE(checkConses(coll, 8u, { coll->getFromRootColor()==1u, coll->getFromColor() == 5u, Color::Void == 2u}));
      }
      WHEN("cycle garbage collector")
      {
        coll->cycleCollector();
        THEN("there is no root and 3 leaf conses")
        {
          REQUIRE(checkConses(coll,
                              8u,
                              { coll->getFromRootColor()==1u,
                                coll->getToColor() == 2u,
                                Color::Void == 5u}));
        }
      }
    }
  }
}

#if 0
SCENARIO("one array without elements", "[ConsColl]")
{
  GIVEN("A root array")
  {
    auto coll = makeCollector(8);
    auto obj = std::make_shared<Object>(coll->makeArray());
    auto array = obj->as<Array>();
    CollectibleGraph graph(*coll);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(array);
      REQUIRE(array->isRoot());
      REQUIRE(array->getColor() == coll->getFromRootColor());
      REQUIRE(array->getRefCount() == 1u);
    }
    THEN("it has no parents")
    {
      arrayHasParents(graph, array);
    }
    THEN("it is reachable")
    {
      REQUIRE(coll->getReachableArraysAsSet() == setOfArrays(array));
    }
    WHEN("the cons is unrooted")
    {
      obj.reset();
      CollectibleGraph graph(*coll);
      THEN("it is a leaf cons with from-color, ref-count 0")
      {
        REQUIRE_FALSE(array->isRoot());
        REQUIRE(array->getColor() == coll->getFromColor());
      }
      THEN("there is no reachable array")
      {
        REQUIRE(coll->getReachableArraysAsSet() == setOfArrays());
      }
    }
  }
}
#endif

SCENARIO("recycle ConsContainer", "[ConsColl]")
{
  auto coll = std::make_shared<GarbageCollector>(8, 1);
  auto container = coll->makeContainer();
  GIVEN("a container with 3 conses")
  {
    Color color = coll->getFromRootColor();
    REQUIRE(container->getColor() == color);
    coll->stepCollector();
    coll->stepCollector();
    REQUIRE(coll->getFromRootColor() !=  color);
    REQUIRE(container->getColor() == coll->getFromRootColor());
    coll->disableCollector();
    container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
    container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
    container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
    REQUIRE((*container)[0]->getColor() == coll->getFromRootColor());
    REQUIRE((*container)[1]->getColor() == coll->getFromRootColor());
    REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
    REQUIRE((*container)[0]->getIndex() == 0);
    REQUIRE((*container)[1]->getIndex() == 1);
    REQUIRE((*container)[2]->getIndex() == 2);
    WHEN("all conses have been marked")
    {
      coll->enableCollector();
      REQUIRE((*container)[0]->getColor() == coll->getFromRootColor());
      REQUIRE((*container)[1]->getColor() == coll->getFromRootColor());
      REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
      REQUIRE(container->getGcTop() == 0);
      coll->stepCollector();
      REQUIRE((*container)[0]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[1]->getColor() == coll->getFromRootColor());
      REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
      REQUIRE(container->getGcTop() == 1);
      coll->stepCollector();
      REQUIRE((*container)[0]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[1]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
      REQUIRE(container->getGcTop() == 2);
      coll->stepCollector();
      REQUIRE((*container)[0]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[1]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[2]->getColor() == coll->getToRootColor());
      REQUIRE(container->getGcTop() == 3);
      color = coll->getFromRootColor();
      THEN("container is marked")
      {
	REQUIRE(container->getColor() == color);
	coll->stepCollector(); // mark container
	REQUIRE(container->getColor() != color);
	REQUIRE(container->getGcTop() == 0);
	coll->stepCollector(); // swap colors
	REQUIRE(container->getGcTop() == 0);
	REQUIRE(coll->getToRootColor() == color);
	REQUIRE((*container)[0]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
      }
      THEN("adding conses to marked container")
      {
	REQUIRE(container->getColor() == color);
	coll->stepCollector(); // mark container
	REQUIRE(container->getColor() != color);
	REQUIRE(container->getColor() == coll->getToRootColor());
	coll->disableCollector();
	container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
	REQUIRE((*container)[0]->getColor() == coll->getToRootColor());
	REQUIRE((*container)[1]->getColor() == coll->getToRootColor());
	REQUIRE((*container)[2]->getColor() == coll->getToRootColor());
	REQUIRE((*container)[3]->getColor() == coll->getToRootColor());
	REQUIRE(container->getGcTop() == 0);
	coll->enableCollector();
	coll->stepCollector(); // swap colors
	REQUIRE(container->getGcTop() == 0);
	REQUIRE(coll->getToRootColor() == color);
	REQUIRE((*container)[0]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[3]->getColor() == coll->getFromRootColor());
      }
    }
    WHEN("part of conses have been marked")
    {
      coll->enableCollector();
      coll->stepCollector();
      coll->stepCollector();
      REQUIRE((*container)[0]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[1]->getColor() == coll->getToRootColor());
      REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
      REQUIRE(container->getGcTop() == 2);
      REQUIRE(container->getColor() == coll->getFromRootColor());
      THEN("adding cons at the end has from color")
      {
	coll->disableCollector();
	container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
	REQUIRE((*container)[3]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == coll->getToRootColor());
	container->setCons(1, coll->makeCons(Lisp::nil, Lisp::nil));
	container->setCons(3, coll->makeCons(Lisp::nil, Lisp::nil));
	REQUIRE((*container)[0]->getColor() == coll->getToRootColor());
	REQUIRE((*container)[1]->getColor() == coll->getToRootColor());
	REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[3]->getColor() == coll->getToRootColor());
        coll->enableCollector();
        coll->stepCollector();
        REQUIRE(container->getGcTop() == 3);
        REQUIRE((*container)[2]->getColor() == coll->getToRootColor());
        REQUIRE(container->getColor() == coll->getFromRootColor());
        color = container->getColor();
        coll->stepCollector();
        REQUIRE(container->getGcTop() == 4);
        REQUIRE(container->getColor() == color);
        coll->stepCollector();
        REQUIRE(container->getColor() != color);
        REQUIRE(container->getColor() == coll->getToRootColor());
        coll->stepCollector();
        REQUIRE(container->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[0]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[1]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[2]->getColor() == coll->getFromRootColor());
	REQUIRE((*container)[3]->getColor() == coll->getFromRootColor());
      }
    }
  }
}

//////////////////////////////////////////////////////////
/// implementation
//////////////////////////////////////////////////////////
static std::shared_ptr<GarbageCollector> makeCollector(std::size_t pageSize)
{
  return std::make_shared<GarbageCollector>(pageSize, 0, 0);
}

static std::size_t getNumCollectible(std::shared_ptr<GarbageCollector> factory)
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
    auto n = getNumCollectible(factory, color);
    if(n == error || n == undef)
    {
      return n;
    }
    ret += n;
  }
  return ret;
}


static std::size_t getNumCollectible(std::shared_ptr<GarbageCollector> factory,
                                     Color color)
{
  if(color != Color::Void)
  {
    std::size_t ret = factory->numCollectible(color);
    bool colorOfConsesEqual = factory->checkSanity(color);
    CHECK(colorOfConsesEqual);
    return colorOfConsesEqual ? ret : error;
  }
  else
  {
    return factory->numVoidCollectible();
  }
}

std::pair<Color, std::size_t> operator==(Color color, std::size_t n)
{
  return std::pair<Color, std::size_t>(color, n);
}

bool checkConses(std::shared_ptr<GarbageCollector> factory,
                 std::size_t total,
                 const std::vector<std::pair<Color, std::size_t> > & conses)
{
  std::size_t num_conses = getNumCollectible(factory);
  CHECK(num_conses == total);
  if(num_conses != total)
  {
    return false;
  }
  return checkConses(factory, conses);
}


bool checkConses(std::shared_ptr<GarbageCollector> factory,
                 const std::vector<std::pair<Color, std::size_t> > & conses)
{
  for(auto p : conses)
  {
    std::size_t num_conses = getNumCollectible(factory, p.first);
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
