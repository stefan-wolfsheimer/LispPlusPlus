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
#include <iostream>
#include <sstream>
#include <catch.hpp>

#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/array.h>
#include <lpp/core/object.h>

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

static std::pair<Color, std::size_t> operator==(Color, std::size_t);

static std::vector<std::pair<Color, std::size_t> > fillMissingColor(const std::vector<std::pair<Color, std::size_t> > & input);
  
static bool checkCollectible(std::shared_ptr<GarbageCollector> factory,
                             const std::vector<std::pair<Color, std::size_t> > & rootConses,
                             const std::vector<std::pair<Color, std::size_t> > & conses);
static bool checkCollectible(std::shared_ptr<GarbageCollector> factory,
                             std::size_t disposed,
                             const std::vector<std::pair<Color, std::size_t> > & rootConses,
                             const std::vector<std::pair<Color, std::size_t> > & conses);

static std::unordered_set<Cell> Set();
static std::unordered_set<Cell> Set(const Cell & c);
template<typename... ARGS>
static std::unordered_set<Cell> Set(const Cell & c, ARGS... rest);
static std::unordered_set<Cell> Set(const std::vector<Cell> & cells);

static std::unordered_set<Cell> Parents(const CollectibleGraph & graph,
                                        const Cell & cell);


SCENARIO("cons_life_cycle", "[GarbageCollector]")
{
  GIVEN("An empty cons factory")
  {
    auto coll = makeCollector(8);
    REQUIRE(checkCollectible(coll, 0u, {}, {}));
    REQUIRE(coll->numDisposedCollectible() == 0u);
    REQUIRE(coll->numVoidCollectible() == 0u);
    REQUIRE(coll->getCycles() == 0);
    WHEN("gc cycle is performed")
    {
      coll->cycle();
      REQUIRE(coll->getCycles() == 1u);
      REQUIRE(coll->numVoidCollectible() == 0u);
      REQUIRE(checkCollectible(coll, 0u, {}, {}));
    }
    WHEN("step is performed")
    {
      coll->enableCollector();
      coll->enableRecycling();
      coll->step();
      REQUIRE(coll->getCycles() == 1u);
      REQUIRE(coll->numVoidCollectible() == 0u);
      REQUIRE(coll->numDisposedCollectible() == 0u);
      REQUIRE(checkCollectible(coll, 0u, {}, {}));
      coll->recycle();
    }
  }
}

SCENARIO("one cons without children", "[GarbageCollector]")
{
  GIVEN("A root cons")
  {
    /*
         +
        / \
    */
    auto coll = makeCollector(8);
    auto obj = std::make_shared<Object>(coll->makeRootCons(Lisp::nil, Lisp::nil));
    auto cons = obj->as<Cons>();
    CollectibleGraph graph(*coll);
    REQUIRE(obj->isRoot());
    REQUIRE(obj->getColor() == Color::White);
    REQUIRE(obj->getRefCount() == 1u);
    REQUIRE(Parents(graph, *obj) == Set());
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons)));
    REQUIRE(coll->numVoidCollectible() == 7u);
    REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u}, {}));

    WHEN("the cons is unrooted")
    {
      obj.reset();
      CollectibleGraph graph(*coll);
      REQUIRE_FALSE(cons->isRoot());
      REQUIRE(cons->getColor() == Color::Grey);
      REQUIRE(coll->numVoidCollectible() == 7u);
      REQUIRE(checkCollectible(coll, 0u, {}, { Color::Grey == 1u }));
      REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set());
      THEN("GarbageCollector::cycle() cleans it up")
      {
        coll->cycle();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(coll->numVoidCollectible() == 8u);
        REQUIRE(checkCollectible(coll, 0u, {}, {}));
      }
      THEN("GarbageCollector::step() cleans it up")
      {
        coll->enableCollector();
        coll->enableRecycling();
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {}, {Color::Black == 1}));
        coll->step();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(checkCollectible(coll, 0u, {}, {Color::White == 1}));
        coll->step();
        REQUIRE(checkCollectible(coll, 1u, {}, {}));
        coll->recycle();
        REQUIRE(checkCollectible(coll, 0u, {}, {}));
      }
    }
  }
}

SCENARIO("one cons with self-ref", "[GarbageCollector]")
{
  GIVEN("A root cons with children referring to itself")
  {
    /*
         +
        / \
    */
    auto coll = makeCollector(8);
    auto obj = std::make_shared<Object>(coll->makeRootCons(Lisp::nil, Lisp::nil));
    auto cons = obj->as<Cons>();
    cons->setCar(*obj);
    cons->setCdr(*obj);
    CollectibleGraph graph(*coll);
    REQUIRE(cons->getCarCell().isA<Cons>());
    REQUIRE(cons->getCdrCell().isA<Cons>());
    REQUIRE(cons->getCarCell().as<Cons>() == cons);
    REQUIRE(cons->getCdrCell().as<Cons>() == cons);
    REQUIRE(cons->getRefCount() == 1u);
    REQUIRE(Parents(graph, *obj) == Set(Cell(cons)));
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons)));
    REQUIRE(coll->numVoidCollectible() == 7u);
    REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {}));
    THEN("GarbageCollector::cycle() turns it white")
    {
      coll->cycle();
      REQUIRE(coll->getCycles() == 1u);
      REQUIRE(coll->numVoidCollectible() == 7u);
      REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {}));
      CollectibleGraph graph(*coll);
      REQUIRE(Parents(graph, *obj) == Set(Cell(cons)));
    }
    THEN("GarbageCollector::step() turns it white")
    {
      coll->enableCollector();
      coll->enableRecycling();
      coll->step();
      REQUIRE(coll->getCycles() == 1u);
      REQUIRE(coll->numVoidCollectible() == 7u);
      REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {}));
      coll->recycle();
      REQUIRE(coll->numVoidCollectible() == 7u);
      REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {}));
      CollectibleGraph graph(*coll);
      REQUIRE(Parents(graph, *obj) == Set(Cell(cons)));
    }
    THEN("unsetting")
    {
      REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {}));
      REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons)));
      obj.reset();
      REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set());
      REQUIRE(checkCollectible(coll, 0u, {}, {Color::Black==1}));
      THEN("GarbageCollector::cycle() removes it")
      {
        coll->cycle();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(coll->numVoidCollectible() == 8u);
        REQUIRE(checkCollectible(coll, 0u, {}, {}));
      }
      THEN("GarbageCollector::step() removes it")
      {
        coll->enableCollector();
        coll->enableRecycling();
        REQUIRE(coll->getCycles() == 0u);
        coll->step();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(coll->numVoidCollectible() == 7u);
        REQUIRE(checkCollectible(coll, 0u, {}, {Color::White==1}));
        coll->step();
        REQUIRE(coll->getCycles() == 2u);
        REQUIRE(coll->numVoidCollectible() == 7u);
        REQUIRE(checkCollectible(coll, 1u, {}, {}));
        coll->recycle();
        REQUIRE(coll->numVoidCollectible() == 8u);
      }
    }
  }
}

SCENARIO("a cons with 2 children", "[GarbageCollector]")
{
  /*
              o
             / \
            o   o 
  */
  GIVEN("A cons coll with 2 children")
  {
    auto coll = makeCollector(8);
    //@todo check if we can use Cell(coll->makeCons)
    auto obj = Object(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil, Lisp::nil)),
                                         Object(coll->makeRootCons(Lisp::nil, Lisp::nil))));
    auto cons = obj.as<Cons>();
    CollectibleGraph graph(*coll);
    REQUIRE(cons);
    REQUIRE(cons->isRoot());
    REQUIRE(cons->getColor() == Color::White);
    REQUIRE(cons->getRefCount() == 1u);
    REQUIRE(Parents(graph, cons) == Set());
    REQUIRE(Parents(graph, cons->getCarCell()) == Set(Cell(cons)));
    REQUIRE(Parents(graph, cons->getCdrCell()) == Set(Cell(cons)));
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons),
                                                                       cons->getCarCell(),
                                                                       cons->getCdrCell()));
    REQUIRE(coll->numVoidCollectible() == 5u);
    REQUIRE(checkCollectible(coll, 0u,
                             { Color::White == 1u},
                             { Color::Grey == 2u}));
    WHEN("There are references to its children")
    {
      {
        auto car = cons->getCar();
        auto cdr = cons->getCdr();
        CollectibleGraph graph(*coll);
        REQUIRE(car.as<Cons>()->getRefCount() == 1u);
        REQUIRE(cdr.as<Cons>()->getRefCount() == 1u);
        REQUIRE(coll->numVoidCollectible() == 5u);
        REQUIRE(checkCollectible(coll, 0u, { Color::White == 3u }, {}));
        REQUIRE(Parents(graph, car) == Set(obj));
        REQUIRE(Parents(graph, cdr) == Set(obj));
      }
      REQUIRE(coll->numVoidCollectible() == 5u);
      REQUIRE(checkCollectible(coll,
                               0u,
                               { Color::White == 1u},
                               { Color::Grey == 2u}));
      THEN("GarbageCollector::cycle() does not remove it")
      {
        coll->cycle();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {Color::White == 2u}));
      }
      THEN("GarbageCollector::step() does not remove it")
      {
        coll->enableCollector();
        coll->enableRecycling();
        REQUIRE(coll->getCycles() == 0u);
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Grey==2}));
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Grey==1, Color::Black==1}));
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Black==2}));
        coll->step();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {Color::White==2}));
      }
      THEN("unsetting one child")
      {
        cons->unsetCar();
        REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons),
                                                                           cons->getCdrCell()));
        REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {Color::Grey==2}));
        THEN("GarbageCollector::cycle() does remove it")
        {
          coll->cycle();
          REQUIRE(coll->getCycles() == 1u);
          REQUIRE(coll->numVoidCollectible() == 6u);
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {Color::White == 1u}));
        }
        THEN("GarbageCollector::step() does remove it")
        {
          REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {Color::Grey==2}));
          coll->enableCollector();
          coll->enableRecycling();
          coll->step();
          REQUIRE(coll->getCycles() == 0u);
          REQUIRE(checkCollectible(coll, 0u, { Color::Black == 1u}, { Color::Grey == 2u}));
          coll->step();
          REQUIRE(coll->getCycles() == 0u);
          REQUIRE(checkCollectible(coll, 0u, { Color::Black == 1u}, { Color::Grey == 1u, Color::Black == 1u}));
          coll->step();
          REQUIRE(checkCollectible(coll, 0u, { Color::Black == 1u}, { Color::Black == 2u}));
          coll->step();
          REQUIRE(coll->getCycles() == 1u);
          REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u}, { Color::White == 2}));
          coll->step();
          REQUIRE(coll->getCycles() == 1u);
          REQUIRE(checkCollectible(coll, 0u, { Color::Black == 1u }, { Color::White == 1, Color::Grey == 1}));
          coll->step();
          REQUIRE(checkCollectible(coll, 0u, { Color::Black == 1u }, { Color::White == 1, Color::Black == 1}));
          coll->step();
          REQUIRE(coll->getCycles() == 2u);
          REQUIRE(checkCollectible(coll, 1u, { Color::White == 1u }, { Color::White == 1}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u }, { Color::White == 1}));
        }
        THEN("setting other child to itself and gc step is executed")
        {
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {Color::Grey == 2u}));
          cons->setCdr(obj);
          REQUIRE(checkCollectible(coll, 0u, {Color::Black == 1u}, {Color::White == 0u, Color::Grey == 2}));
          coll->enableCollector();
          coll->enableRecycling();
          coll->step();
          REQUIRE(checkCollectible(coll, 0u, {Color::Black == 1u}, {Color::Black == 1u, Color::Grey == 1}));
          coll->step();
          REQUIRE(checkCollectible(coll, 0u, {Color::Black == 1u}, {Color::Black == 2u}));
          coll->step();
          REQUIRE(coll->getCycles() == 1u);
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {Color::White == 2u}));
          coll->step();
          REQUIRE(coll->getCycles() == 1u);
          REQUIRE(checkCollectible(coll, 0u, {Color::Black == 1u}, {Color::White == 2u}));
          coll->step();
          REQUIRE(coll->getCycles() == 2u);
          REQUIRE(checkCollectible(coll, 2u, {Color::White == 1u}, {}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 1u, {Color::White == 1u}, {}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {}));
        }
      }
    }
  }
}
 
SCENARIO("3 conses with 4 children", "[GarbageCollector]")
{
  GIVEN("2 conses")
  {
    /*
           o        o 
          / \      / \
         o   o    o   o
      */

    auto coll = makeCollector(8);
    Object cons1(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil)),
                                    Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil))));
    Object cons2(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil)),
                                    Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil))));
    CollectibleGraph graph(*coll);
    REQUIRE(coll->numVoidCollectible() == 2u);
    REQUIRE(checkCollectible(coll, 0u,
                             { Color::White == 2u},
                             { Color::Grey == 4u}));
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(cons1,
                                                                       cons1.as<Cons>()->getCarCell(),
                                                                       cons1.as<Cons>()->getCdrCell(),
                                                                       cons2,
                                                                       cons2.as<Cons>()->getCarCell(),
                                                                       cons2.as<Cons>()->getCdrCell()));
    WHEN("there are 2 conses with 4 children")
    {
      /*
           o        o    o
          / \      / \  / \
         o   o    o   o
      */

      Object cons3(coll->makeRootCons(cons2.as<Cons>()->getCar(), Lisp::nil));
      CollectibleGraph graph(*coll);
      REQUIRE(coll->numVoidCollectible() == 1u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::White == 3u},
                               { Color::Grey == 4u}));
      REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(cons1,
                                                                         cons1.as<Cons>()->getCarCell(),
                                                                         cons1.as<Cons>()->getCdrCell(),
                                                                         cons2,
                                                                         cons2.as<Cons>()->getCarCell(),
                                                                         cons2.as<Cons>()->getCdrCell(),
                                                                         cons3,
                                                                         cons3.as<Cons>()->getCarCell()));
      WHEN("cons2 is unset")
      {
        /*
                o            o
               / \          / \
              o   o        o
        */
        cons2 = Lisp::nil;
        CollectibleGraph graph(*coll);
        REQUIRE(coll->numVoidCollectible() == 1u);
        REQUIRE(checkCollectible(coll, 0u,
                                 { Color::White == 2u },
                                 { Color::Grey == 5u }));
        REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(cons1,
                                                                           cons1.as<Cons>()->getCarCell(),
                                                                           cons1.as<Cons>()->getCdrCell(),
                                                                           cons3.as<Cons>(),
                                                                           cons3.as<Cons>()->getCarCell()));
        REQUIRE(coll->getCycles() == 0);
        coll->cycle();
        REQUIRE(coll->getCycles() == 1);
        REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(cons1,
                                                                           cons1.as<Cons>()->getCarCell(),
                                                                           cons1.as<Cons>()->getCdrCell(),
                                                                           cons3.as<Cons>(),
                                                                           cons3.as<Cons>()->getCarCell()));
        REQUIRE(checkCollectible(coll, 0u,
                                 { Color::White == 2u },
                                 { Color::White == 3u }));
        REQUIRE(coll->numVoidCollectible() == 3u);
      }
    }
  }
}

SCENARIO("copy cons object with object copy constructor", "[GarbageCollector]")
{
  GIVEN("A cons with 2 children")
  {
    auto coll = makeCollector(8);
    Object cons1(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil)),
                                    Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil))));
    REQUIRE(cons1.isA<Cons>());
    REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
    WHEN("there is a copy of the cons")
    {
      Object cons2(cons1);
      REQUIRE(cons2.isA<Cons>());
      REQUIRE(cons2.as<Cons>() == cons1.as<Cons>());
      REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
      REQUIRE(coll->numVoidCollectible() == 5u);
      REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u}, { Color::Grey == 2u}));
      WHEN("copy is unset")
      {
        cons2 = Lisp::nil;
        REQUIRE(cons1.as<Cons>()->getRefCount() == 1u);
        REQUIRE(coll->numVoidCollectible() == 5u);
        REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u}, { Color::Grey == 2u}));
        WHEN("both conses are unset")
        {
          cons1 = Lisp::nil;
          REQUIRE(cons1.isA<Nil>());
          REQUIRE(coll->numVoidCollectible() == 5u);
          REQUIRE(checkCollectible(coll, 0u, { Color::White == 0u}, { Color::Grey == 3u}));
          WHEN("cycle garbage location")
          {
            coll->cycle();
            REQUIRE(coll->numVoidCollectible() == 8u);
          }
        }
      }
    }
  }
}

SCENARIO("copy cons object with object assignement operator", "[GarbageCollector]")
{
  GIVEN("conses with 4 children")
  {
    auto coll = makeCollector(8);
    Object cons1(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil)),
                                    Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil))));
    Object cons2(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil)),
                                    Object(coll->makeRootCons(Lisp::nil,
                                                              Lisp::nil))));
    WHEN("cons1 is set to cons2")
    {
      REQUIRE(cons2.as<Cons>()->getRefCount() == 1u);
      cons1 = cons2;
      REQUIRE(cons2.as<Cons>()->getRefCount() == 2u);
      REQUIRE(coll->numVoidCollectible() == 2u);
      REQUIRE(checkCollectible(coll,
                               0u,
                               { Color::White == 1u},
                               { Color::Grey == 5u}));
      WHEN("cycle garbage collector")
      {
        coll->cycle();
        REQUIRE(coll->numVoidCollectible() == 5u);
        REQUIRE(checkCollectible(coll, 0u,
                                 { Color::White == 1u},
                                 { Color::White == 2u}));
      }
    }
  }
}

TEST_CASE("automatic collection with lots of temporary objects", "[GarbageCollector]")
{
  auto coll = makeCollector(512);
  coll->enableCollector();
  coll->enableRecycling();
  {
    auto obj = Object(coll->makeRootCons(Lisp::nil,
                                         Object(coll->makeRootCons(Lisp::nil,
                                                                   Object(coll->makeRootCons(Lisp::nil,
                                                                                             Object(coll->makeRootCons(Lisp::nil,
                                                                                                                       Lisp::nil))))))));
    REQUIRE(coll->getCycles() == 2u);
    REQUIRE(checkCollectible(coll, 0u,
                             { Color::Black == 1u},
                             { Color::White == 1u, Color::Grey == 2u}));
    
    REQUIRE(coll->checkSanity());
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(obj,
                                                                       obj.as<Cons>()->getCdrCell(),
                                                                       obj.as<Cons>()->getCdrCell().as<Cons>()->getCdrCell(),
                                                                       obj.as<Cons>()->getCdrCell().as<Cons>()->getCdrCell().as<Cons>()->getCdrCell()));
    REQUIRE(obj.as<Cons>()->getCar().isA<Nil>());
    REQUIRE(coll->checkSanity());
    REQUIRE(obj.as<Cons>()->getCdr().isA<Cons>());
    REQUIRE(coll->checkSanity());
    REQUIRE(obj.as<Cons>()
            ->getCdr().as<Cons>()
            ->getCar().isA<Nil>());
    REQUIRE(coll->checkSanity());
    REQUIRE(obj.as<Cons>()
            ->getCdr().as<Cons>()
            ->getCdr().as<Cons>()
            ->getCar().isA<Nil>());
    REQUIRE(coll->checkSanity());
    REQUIRE(obj.as<Cons>()
            ->getCdr().as<Cons>()
            ->getCdr().as<Cons>()
            ->getCdr().as<Cons>()
            ->getCar().isA<Nil>());

  }
  REQUIRE(coll->checkSanity());
}

////////////////////////////////////////////////////////////////////////////////
//
// Array tests
//
////////////////////////////////////////////////////////////////////////////////
SCENARIO("one array with cons children", "[GarbageCollector]")
{
  auto coll = makeCollector(8);
  GIVEN("A root array")
  {
    auto cons1 = Object(coll->makeRootCons(Object(coll->makeRootCons(Lisp::nil, Lisp::nil)),
                                           Object(coll->makeRootCons(Lisp::nil, Lisp::nil))));
    REQUIRE(cons1.getColor() == Color::White);
    REQUIRE(cons1.as<Cons>()->getCarCell().getColor() == Color::Grey);
    REQUIRE(cons1.as<Cons>()->getCarCell().getColor() == Color::Grey);
    auto obj = Object(coll->makeRoot<Array>());
    auto array = obj.as<Array>();
    REQUIRE(array->isRoot());
    REQUIRE(array->getColor() == Color::White);
    REQUIRE(array->getRefCount() == 1u);
    array->append(Cell(coll->makeCons(Lisp::nil, Lisp::nil)),
                  Object(1),
                  Cell(coll->makeCons(Lisp::nil, Lisp::nil)),
                  Object(3),
                  coll->makeCons(Lisp::nil, Lisp::nil));
    REQUIRE(array->atCell(0).isA<Cons>());
    REQUIRE(array->atCell(0).isRoot() == false);
    REQUIRE(array->atCell(0).getColor() == Color::Grey);
    REQUIRE(array->atCell(2).isA<Cons>());
    REQUIRE(array->atCell(2).isRoot() == false);
    REQUIRE(array->atCell(2).getColor() == Color::Grey);
    REQUIRE(array->atCell(4).isA<Cons>());
    REQUIRE(array->atCell(4).isRoot() == false);
    REQUIRE(array->atCell(4).getColor() == Color::Grey);
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(array),
                                                                       array->atCell(0),
                                                                       array->atCell(2),
                                                                       array->atCell(4),
                                                                       cons1,
                                                                       cons1.as<Cons>()->getCarCell(),
                                                                       cons1.as<Cons>()->getCdrCell()));
    WHEN("There are references to its children")
    {
      {
        auto child2 = array->at(0);
        auto child4 = array->at(4);
        CollectibleGraph graph(*coll);
        REQUIRE(array->atCell(0).isRoot());
        REQUIRE(array->atCell(4).isRoot());
        REQUIRE(array->atCell(0).getRefCount() == 1u);
        REQUIRE(array->atCell(4).getRefCount() == 1u);
        REQUIRE(checkCollectible(coll, 0u, { Color::White == 4u }, { Color::Grey == 3u}));
        REQUIRE(Parents(graph, child2) == Set(obj));
        REQUIRE(Parents(graph, child4) == Set(obj));
      }
      REQUIRE(array->atCell(0).isRoot() == false);
      REQUIRE(array->atCell(4).isRoot() == false);
      REQUIRE(array->atCell(0).getRefCount() == 0u);
      REQUIRE(array->atCell(4).getRefCount() == 0u);
      REQUIRE(checkCollectible(coll, 0u, { Color::White == 2u }, { Color::Grey == 5u}));
    }
    WHEN("collected stepwise")
    {
      REQUIRE(array->getGcPosition() == 0u);
      coll->enableCollector();
      coll->enableRecycling();

      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 1u);
      REQUIRE(cons1.getColor() == Color::Black);
      REQUIRE(cons1.as<Cons>()->getCarCell().getColor() == Color::Grey);
      REQUIRE(cons1.as<Cons>()->getCdrCell().getColor() == Color::Grey);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::White == 1u, Color::Black == 1u },
                               { Color::White == 0u, Color::Grey == 5u }));
      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 2u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::White == 1u, Color::Black == 1u },
                               { Color::Black == 1u, Color::Grey == 4u}));
      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 3u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::White == 1u, Color::Black == 1u },
                               { Color::Black == 2u, Color::Grey == 3u}));
      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 4u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::White == 1u, Color::Black == 1u },
                               { Color::Black == 3u, Color::Grey == 2u }));

      /////////////////////////
      coll->disableCollector();
      array->set(1, Object(coll->makeRootCons(Lisp::nil, Lisp::nil)));
      coll->enableCollector();
      REQUIRE(array->atCell(1).getColor() == Color::Grey);
      
      
      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 0u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::Black == 2u },
                               { Color::Black == 4u, Color::Grey == 2u }));

      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 0u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::Black == 2u },
                               { Color::Black == 5u, Color::Grey == 1u }));

      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 0u);
      REQUIRE(array->getGcPosition() == 0u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::Black == 2u },
                               { Color::Black == 6u }));

      //////////////////////////
      coll->step();
      REQUIRE(coll->getCycles() == 1u);
      REQUIRE(array->getGcPosition() == 0u);
      REQUIRE(checkCollectible(coll, 0u,
                               { Color::White == 2u },
                               { Color::White == 6u }));

      //////////////////////////
      obj = Lisp::nil;
      cons1 = Lisp::nil;
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::White == 2, Color::Grey == 4, Color::Black == 2 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::White == 1, Color::Grey == 4, Color::Black == 3 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::White == 1, Color::Grey == 3, Color::Black == 4 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::Grey == 2, Color::Black == 6 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::Grey == 1, Color::Black == 7 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::Black == 8 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 0u,
                               { },
                               { Color::White == 8 }));
      coll->step();
      REQUIRE(checkCollectible(coll, 8u, {}, {}));

      for(std::size_t i = 0; i < 8u; ++i)
      {
        coll->recycle();
      }
      REQUIRE(checkCollectible(coll, 0u, {}, {}));
    }
    WHEN("recycled")
    {
      obj = Lisp::nil;
      cons1 = Lisp::nil;
      REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set());
      REQUIRE(array->getGcPosition() == 0u);
      coll->enableCollector();
      coll->enableRecycling();
      coll->cycle();
      REQUIRE(coll->getCycles() == 1u);
      REQUIRE(checkCollectible(coll, {}, {}));
    }
  }
}

SCENARIO("a cons with 2 array children", "[GarbageCollector]")
{
  /*
              o
             / \
            o   o 
  */
  GIVEN("A cons coll with 2 children")
  {
    auto coll = makeCollector(8);
    auto obj = Object(coll->makeRootCons(Cell(coll->make<Array>(Lisp::nil, Lisp::nil)),
                                         Cell(coll->make<Array>(Lisp::nil, Lisp::nil))));
    auto cons = obj.as<Cons>();
    REQUIRE(cons->isRoot());
    REQUIRE(cons->getColor() == Color::White);
    REQUIRE(cons->getRefCount() == 1u);

    CollectibleGraph graph(*coll);
    REQUIRE(cons);
    REQUIRE(cons->isRoot());
    REQUIRE(cons->getColor() == Color::White);
    REQUIRE(cons->getRefCount() == 1u);
    REQUIRE(Parents(graph, cons) == Set());
    REQUIRE(Parents(graph, cons->getCarCell()) == Set(Cell(cons)));
    REQUIRE(Parents(graph, cons->getCdrCell()) == Set(Cell(cons)));
    REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons),
                                                                       cons->getCarCell(),
                                                                       cons->getCdrCell()));
    REQUIRE(coll->numVoidCollectible() == 7u);
    REQUIRE(checkCollectible(coll, 0u,
                             { Color::White == 1u},
                             { Color::White == 2u}));

    WHEN("There are references to its children")
    {
      {
        auto car = cons->getCar();
        auto cdr = cons->getCdr();
        CollectibleGraph graph(*coll);
        REQUIRE(car.as<Array>()->getRefCount() == 1u);
        REQUIRE(cdr.as<Array>()->getRefCount() == 1u);
        REQUIRE(coll->numVoidCollectible() == 7u);
        REQUIRE(checkCollectible(coll, 0u, { Color::White == 3u }, {}));
        REQUIRE(Parents(graph, car) == Set(obj));
        REQUIRE(Parents(graph, cdr) == Set(obj));
      }
      REQUIRE(coll->numVoidCollectible() == 7u);
      REQUIRE(checkCollectible(coll,
                               0u,
                               { Color::White == 1u},
                               { Color::Grey == 2u}));
      THEN("GarbageCollector::cycle() does not remove it")
      {
        coll->cycle();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {Color::White == 2u}));
      }
      THEN("GarbageCollector::step() does not remove it")
      {
        coll->enableCollector();
        coll->enableRecycling();
        REQUIRE(coll->getCycles() == 0u);
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Grey==2}));
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Grey==1, Color::Black==1}));
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Black==1, Color::Grey==1}));
        coll->step();
        REQUIRE(coll->getCycles() == 0u);
        REQUIRE(checkCollectible(coll, 0u, {Color::Black==1}, {Color::Black==2}));
        coll->step();
        REQUIRE(coll->getCycles() == 1u);
        REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {Color::White==2}));
      }  
      THEN("unsetting one child")
      {
        cons->unsetCar();
        REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons),
                                                                           cons->getCdrCell()));
        REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {Color::Grey==2}));
        THEN("GarbageCollector::cycle() does remove it")
        {
          coll->cycle();
          REQUIRE(coll->getCycles() == 1u);
          REQUIRE(coll->numVoidCollectible() == 7u);
          REQUIRE(checkCollectible(coll, 1u, {Color::White == 1u}, {Color::White == 1u}));
        }
        THEN("GarbageCollector::step() does remove it")
        {
          REQUIRE(checkCollectible(coll, 0u, {Color::White==1}, {Color::Grey==2}));
          coll->enableCollector();
          coll->enableRecycling();

          for(std::size_t c = coll->getCycles(); coll->getCycles() == c;)
          {
            coll->step();
          }
          REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u }, { Color::White == 2}));
          for(std::size_t c = coll->getCycles(); coll->getCycles() == c;)
          {
            coll->step();
          }
          REQUIRE(checkCollectible(coll, 1u, { Color::White == 1u }, { Color::White == 1}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 0u, { Color::White == 1u }, { Color::White == 1}));
        }
        THEN("setting other child to itself and gc step is executed")
        {
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {Color::Grey == 2u}));
          cons->setCdr(obj);
          REQUIRE(Set(coll->get(&GarbageCollector::forEachReachable)) == Set(Cell(cons)));

          REQUIRE(checkCollectible(coll, 0u, {Color::Black == 1u}, {Color::White == 0u, Color::Grey == 2}));
          coll->enableCollector();
          coll->enableRecycling();

          for(std::size_t c = coll->getCycles(); coll->getCycles() == c;)
          {
            coll->step();
          }
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, { Color::White == 2}));
          for(std::size_t c = coll->getCycles(); coll->getCycles() == c;)
          {
            coll->step();
          }
          REQUIRE(coll->getCycles() == 2u);
          REQUIRE(checkCollectible(coll, 2u, {Color::White == 1u}, {}));
          REQUIRE(checkCollectible(coll, 2u, {Color::White == 1u}, {}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 1u, {Color::White == 1u}, {}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 1u, {Color::White == 1u}, {}));
          coll->recycle();
          REQUIRE(checkCollectible(coll, 0u, {Color::White == 1u}, {}));
        } 
      }
    } 
  }
}


//////////////////////////////////////////////////////////
/// implementation
//////////////////////////////////////////////////////////
static std::shared_ptr<GarbageCollector> makeCollector(std::size_t pageSize)
{
  auto ret = std::make_shared<GarbageCollector>(pageSize, 1, 1);
  ret->disableRecycling();
  ret->disableCollector();
  return ret;
}

std::pair<Color, std::size_t> operator==(Color color, std::size_t n)
{
  return std::pair<Color, std::size_t>(color, n);
}

bool checkCollectible(std::shared_ptr<GarbageCollector> coll,
                      std::size_t disposed,
                      const std::vector<std::pair<Color, std::size_t> > & root,
                      const std::vector<std::pair<Color, std::size_t> > & bulk)
{
  CHECK(coll->numDisposedCollectible() == disposed);
  if(coll->numDisposedCollectible() != disposed)
  {
    return false;
  }
  return checkCollectible(coll, root, bulk);
}

static std::vector<std::pair<Color, std::size_t> > fillMissingColor(const std::vector<std::pair<Color, std::size_t> > & input)
{
  std::size_t white = 0;
  std::size_t black = 0;
  std::size_t grey = 0;
  for(auto p : input)
  {
    switch(p.first)
    {
    case Color::White: white = p.second; break;
    case Color::Black: black = p.second; break;
    case Color::Grey:  grey = p.second; break;
    }
  }
  return std::vector<std::pair<Color, std::size_t> >({
      Color::White == white, Color::Grey == grey, Color::Black == black});
}

static std::string asString(const std::vector<std::pair<Color, std::size_t> > & input)
{
  std::stringstream ss;
  ss << "{";
  bool first = true;
  for(auto p : input)
  {
    if(!first) ss << ",";
    else first = false;
    ss << p.first << ":" << p.second;
  }
  ss << "}";
  return ss.str();
}

bool checkCollectible(std::shared_ptr<GarbageCollector> coll,
                      const std::vector<std::pair<Color, std::size_t> > & root,
                      const std::vector<std::pair<Color, std::size_t> > & bulk)
{
  auto givenRoot = asString(std::vector<std::pair<Color, std::size_t> >({
        Color::White == coll->numRootCollectible(Color::White),
        Color::Grey  == coll->numRootCollectible(Color::Grey),
        Color::Black == coll->numRootCollectible(Color::Black) }));
  auto givenBulk = asString(std::vector<std::pair<Color, std::size_t> >({
       Color::White == coll->numBulkCollectible(Color::White),
       Color::Grey  == coll->numBulkCollectible(Color::Grey),
       Color::Black == coll->numBulkCollectible(Color::Black)}));

  CHECK(coll->checkRootSanity(Color::Black));
  CHECK(coll->checkRootSanity(Color::Grey));
  CHECK(coll->checkRootSanity(Color::White));
  CHECK(coll->checkBulkSanity(Color::Black));
  CHECK(coll->checkBulkSanity(Color::Grey));
  CHECK(coll->checkBulkSanity(Color::White));
  CHECK(givenRoot == asString(fillMissingColor(root)));
  CHECK(givenBulk == asString(fillMissingColor(bulk)));
  return
    coll->checkRootSanity(Color::Black) &
    coll->checkRootSanity(Color::Grey) &
    coll->checkRootSanity(Color::White) &
    coll->checkBulkSanity(Color::Black) &
    coll->checkBulkSanity(Color::Grey) &
    coll->checkBulkSanity(Color::White) &
    ( givenRoot == asString(fillMissingColor(root))) &
    ( givenBulk == asString(fillMissingColor(bulk)));
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
