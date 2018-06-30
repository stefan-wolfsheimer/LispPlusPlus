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
    auto obj = std::make_shared<Object>(coll->makeCons(Lisp::nil, Lisp::nil));
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
    auto obj = std::make_shared<Object>(coll->makeCons(Lisp::nil, Lisp::nil));
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
    auto obj = std::make_shared<Object>(coll->makeCons(Object(coll->makeCons(Lisp::nil, Lisp::nil)),
                                                       Object(coll->makeCons(Lisp::nil, Lisp::nil))));
    auto cons = obj->as<Cons>();
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
          cons->setCdr(*obj);
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
    Object cons1(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                      Lisp::nil)),
                                Object(coll->makeCons(Lisp::nil,
                                                      Lisp::nil))));
    Object cons2(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                      Lisp::nil)),
                                Object(coll->makeCons(Lisp::nil,
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

      Object cons3(coll->makeCons(cons2.as<Cons>()->getCar(), Lisp::nil));
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
    Object cons1(coll->makeCons(Object(coll->makeCons(Lisp::nil,
                                                      Lisp::nil)),
                                Object(coll->makeCons(Lisp::nil,
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

SCENARIO("one array without elements", "[GarbageCollector]")
{
  /*@todo implement Array */
  auto coll = makeCollector(8);
  GIVEN("A root array")
  {
    auto obj = Object(coll->make<Array>());
    auto array = obj.as<Array>();
    CollectibleGraph graph(*coll);
    REQUIRE(array->isRoot());
    REQUIRE(array->getColor() == Color::White);
    REQUIRE(array->getRefCount() == 1u);
    array->append(coll->makeCons(Lisp::nil, Lisp::nil),
                  Object(1),
                  coll->makeCons(Lisp::nil, Lisp::nil),
                  Object(3),
                  coll->makeCons(Lisp::nil, Lisp::nil));
    REQUIRE(array->at(0).isA<Cons>());
    REQUIRE(array->at(0).isRoot() == false);
    REQUIRE(array->at(0).getColor() == Color::Grey);
    REQUIRE(array->at(2).isA<Cons>());
    REQUIRE(array->at(2).isRoot() == false);
    REQUIRE(array->at(2).getColor() == Color::Grey);
    REQUIRE(array->at(4).isA<Cons>());
    REQUIRE(array->at(4).isRoot() == false);
    REQUIRE(array->at(4).getColor() == Color::Grey);
  }
}

TEST_CASE("automatic collection with lots of temporary objects", "[GarbageCollector]")
{
  auto coll = makeCollector(512);
  coll->enableCollector();
  coll->enableRecycling();
  {
    auto obj = Object(coll->makeCons(Lisp::nil,
                                     Object(coll->makeCons(Lisp::nil,
                                                           Object(coll->makeCons(Lisp::nil,
                                                                                 Object(coll->makeCons(Lisp::nil,
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

#if 0
SCENARIO("one array without elements", "[GarbageCollector]")
{
  GIVEN("A root array")
  {
    CollectibleGraph graph(*coll);
    THEN("it is in root set, has from-color and ref count 1")
    {
      REQUIRE(array);
      REQUIRE(array->isRoot());
      REQUIRE(array->getColor() == Color::White);
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
        REQUIRE(array->getColor() == Color::White);
      }
      THEN("there is no reachable array")
      {
        REQUIRE(coll->getReachableArraysAsSet() == setOfArrays());
      }
    }
  }
}


SCENARIO("recycle ConsContainer", "[GarbageCollector]")
{
  auto coll = std::make_shared<GarbageCollector>(8, 1);
  auto container = coll->makeContainer();
  GIVEN("a container with 3 conses")
  {
    REQUIRE(container->getColor() == Color::White);
    coll->stepCollector();
    coll->stepCollector();
    REQUIRE(container->getColor() == Color::White);
    coll->disableCollector();
    container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
    container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
    container->pushCons(coll->makeCons(Lisp::nil, Lisp::nil));
    REQUIRE((*container)[0]->getColor() == Color::White);
    REQUIRE((*container)[1]->getColor() == Color::White);
    REQUIRE((*container)[2]->getColor() == Color::White);
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
#endif

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
  bool sanity = true;
  std::vector<std::pair<Color, std::size_t> >givenRoot = {
    Color::White == coll->numRootCollectible(Color::White),
    Color::Grey  == coll->numRootCollectible(Color::Grey),
    Color::Black == coll->numRootCollectible(Color::Black) };
  std::vector<std::pair<Color, std::size_t> >givenBulk = {
    Color::White == coll->numBulkCollectible(Color::White),
    Color::Grey  == coll->numBulkCollectible(Color::Grey),
    Color::Black == coll->numBulkCollectible(Color::Black) };

  CHECK((sanity &= coll->checkRootSanity(Color::Black)));
  CHECK((sanity &= coll->checkRootSanity(Color::Grey)));
  CHECK((sanity &= coll->checkRootSanity(Color::White)));
  CHECK((sanity &= coll->checkBulkSanity(Color::Black)));
  CHECK((sanity &= coll->checkBulkSanity(Color::Grey)));
  CHECK((sanity &= coll->checkBulkSanity(Color::White)));
  CHECK((sanity &= ( asString(givenRoot) == asString(fillMissingColor(root)))));
  CHECK((sanity &= ( asString(givenBulk) == asString(fillMissingColor(bulk)))));
  return sanity;
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
