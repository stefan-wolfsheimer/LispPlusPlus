/******************************************************************************
Copyright (c) 2017-2018, Stefan Wolfsheimer

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
#include <vector>
#include <unordered_set>
#include "lisp_cons_factory.h"
#include "lisp_cons.h"
#include "lisp_cons_graph.h"
#include "lisp_cons_graph_node.h"
#include "lisp_cons_graph_edge.h"

// helper types
using ConsFactory = Lisp::ConsFactory;
using Cons = Lisp::Cons;
using SharedConsFactory = std::shared_ptr<ConsFactory>;
using ConsGraph = Lisp::ConsGraph;
using Object = Lisp::Object;

// helper constants
static const std::size_t undef = std::numeric_limits<std::size_t>::max();

// helper functions
static SharedConsFactory makeFactory(std::size_t pageSize=12);
static std::size_t getWeight(const ConsGraph & graph, const Cons * parent, const Cons * child);
static bool consHasParents(const ConsGraph & graph, const Cons * cons,  const std::vector<const Cons*> & parents);
static bool consHasParents(const ConsGraph & graph, const Cons * cons);
static bool consHasParents(const ConsGraph & graph, const Cons * cons, const Cons* p1);

SCENARIO("cons graph consitency", "[ConsGraph]")
{
  GIVEN("A cons factory")
  {
    auto factory = makeFactory(8);
    WHEN("One Cons without children is allocated")
    {
      Cons * pcons = factory->make(Lisp::nil, Lisp::nil);
      ConsGraph graph(*factory);
      THEN("the edge weight from root to cons is 1")
      {
        REQUIRE(getWeight(graph, nullptr, pcons) == 1u);
      }
      THEN("the cons has no parents")
      {
        consHasParents(graph, pcons);
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
      ConsGraph graph(*factory);
      THEN("Parent of children is cons")
      {
        consHasParents(graph, pcons->getCarCell().as<Cons>(), pcons);
        consHasParents(graph, pcons->getCdrCell().as<Cons>(), pcons);
      }
      THEN("the edge weight from cons to cons is undef")
      {
        REQUIRE(getWeight(graph, pcons, pcons) == undef);
      }
      THEN("the edge weight from root to cons is 2")
      {
        REQUIRE(getWeight(graph, nullptr, pcons) == 3u);
      }
      THEN("the edge weight from cons to children is 1")
      {
        REQUIRE(getWeight(graph, pcons, pcons->getCarCell().as<Cons>()) == 1u);
        REQUIRE(getWeight(graph, pcons, pcons->getCdrCell().as<Cons>()) == 1u);
      }
    }
  }
}

//////////////////////////
// details
//////////////////////////
static SharedConsFactory makeFactory(std::size_t pageSize)
{
  return std::make_shared<ConsFactory>(pageSize, 0, 0);
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

static bool consHasParents(const ConsGraph & graph, const Cons * cons,  const std::vector<const Cons*> & parents)
{
  auto node = graph.findNode(cons);
  if(node)
  {
    return node->getConsParents() == std::unordered_set<const Cons*>(parents.begin(), parents.end());
  }
  else
  {
    return false;
  }
}

static bool consHasParents(const ConsGraph & graph, const Cons * cons)
{
  return consHasParents(graph, cons, std::vector<const Cons*>());
}

static bool consHasParents(const ConsGraph & graph, const Cons * cons, const Cons* p1)
{
  return consHasParents(graph, cons, std::vector<const Cons*>({p1}));
}
