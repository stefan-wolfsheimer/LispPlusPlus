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
#include <lpp/simul/gc_sim.h>
#include <catch.hpp>

using GcSim = Lisp::GcSim;
using Cell = Lisp::Cell;

static std::set<std::size_t> Parents(const GcSim & gcSim, std::size_t index);
static std::set<std::size_t> Set() { return std::set<std::size_t>(); }

template<typename... A>
static std::set<std::size_t> Set(std::size_t i, A... rest) { std::set<std::size_t> s = Set(rest...); s.insert(i); return s; }


TEST_CASE("gc_sim_cycle", "[GcSim]")
{
  GcSim sim;
  sim.disableCollector();
  REQUIRE(sim.numRoot() == 0);
  REQUIRE(sim.numBulk() == 0);
  REQUIRE(sim.numChildren() == 0);
  REQUIRE(sim.numFreeChildren() == 0);
  REQUIRE(sim.numEdges() == 0);
  REQUIRE(sim.checkSanity());

  REQUIRE(sim.addRoot(2) == 0);
  /*
   * containers       freeChildren
   * 0: cons          0: (0, 0) -> null
   *                  1: (0, 1) -> null
   *
   * edges:
   * 0: R -> 0
   * 
   */
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(sim.numBulk() == 0);
  REQUIRE(sim.numRoot() == 1);
  REQUIRE(sim.numChildren() == 0);
  REQUIRE(sim.numFreeChildren() == 2);
  REQUIRE(sim.numEdges() == 1);
  REQUIRE(sim.checkSanity());

  REQUIRE(sim.addBulk(std::make_pair(0, 0), 0) == 1);
  /*
   * containers       freeChildren         children
   * 0: cons          0: (0, 1) -> null    0: (0, 0) -> 1     
   * 1: array         
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: R -> 0
   * 
   */
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(sim.numChildren() == 1);
  REQUIRE(sim.numFreeChildren() == 1);
  REQUIRE(sim.numEdges() == 2);
  REQUIRE(sim.checkSanity());

  REQUIRE(sim.addBulk(std::make_pair(0, 1), 3) == 2);
    /*
   * containers       freeChildren         children
   * 0: cons(0,1)                          0: (0, 0) -> 1
   *                                       1: (0, 1) -> 2
   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: R -> 0
   * 
   */
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(Parents(sim, 2) == Set(0));

  REQUIRE(sim.numRoot() == 1);
  REQUIRE(sim.numBulk() == 2);
  REQUIRE(sim.numChildren() == 2);
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 3);

  REQUIRE(sim.addRoot(1) == 3);
  /*
   * containers       freeChildren         children
   * 0: cons                               0: (0, 0) -> 1
   *                                       1: (0, 1) -> 2
   * 1: array 
   * 2: array         0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3: array         3: (3, 0) -> null
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: R -> 0
   * 3: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(Parents(sim, 2) == Set(0));
  REQUIRE(Parents(sim, 3) == Set());

  REQUIRE(sim.numRoot() == 2);
  REQUIRE(sim.numBulk() == 2);
  REQUIRE(sim.numChildren() == 2);
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numEdges() == 4);
  REQUIRE(sim.checkSanity());

  REQUIRE(sim.addBulk(std::make_pair(3, 0), 2) == 4);
  /*
   * containers       freeChildren         children
   * 0: cons                               0: (0, 0) -> 1
   *                                       1: (0, 1) -> 2
   * 1: array 
   * 2: arra          0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3: array                              2: (3, 0) -> 4
   * 4: cons          3: (4, 0) -> null
   *                  4: (4, 1) -> null
   * 
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: R -> 0
   * 4: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(Parents(sim, 2) == Set(0));
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set(3));
  REQUIRE(sim.numRoot() == 2);
  REQUIRE(sim.numBulk() == 3);
  REQUIRE(sim.numChildren() == 3);
  REQUIRE(sim.numFreeChildren() == 5);
  REQUIRE(sim.numEdges() == 5);
  REQUIRE(sim.checkSanity());

  sim.addEdge(std::make_pair(4, 1), 0);
  /*
   * containers       freeChildren         children
   * 0: cons                               0: (0, 0) -> 1
   *                                       1: (0, 1) -> 2
   * 1: array 
   * 2: arra          0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3: array                              2: (3, 0) -> 4
   * 4: cons          3: (4, 0) -> null    3: (4, 1) -> 0
   * 
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: R -> 0
   * 5: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set(4));
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(Parents(sim, 2) == Set(0));
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set(3));

  REQUIRE(sim.numRoot() == 2);
  REQUIRE(sim.numBulk() == 3);
  REQUIRE(sim.numChildren() == 4);
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numEdges() == 6);
  REQUIRE(sim.checkSanity());

  sim.addEdge(std::make_pair(4, 0), 4);
  /*
   * containers       freeChildren         children
   * 0: cons                               0: (0, 0) -> 1
   *                                       1: (0, 1) -> 2
   * 1: array 
   * 2: arra          0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3: array                              2: (3, 0) -> 4
   * 4: cons                               3: (4, 1) -> 0
   *                                       4: (4, 0) -> 4
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   * 5: R -> 0
   * 6: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set(4));
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(Parents(sim, 2) == Set(0));
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set(3, 4));

  REQUIRE(sim.numRoot() == 2);
  REQUIRE(sim.numBulk() == 3);
  REQUIRE(sim.numChildren() == 5);
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 7);
  REQUIRE(sim.checkSanity());

  sim.removeEdge(std::make_pair(GcSim::ROOT(), 0));
  /*
   * containers       freeChildren         children
   * 0: cons                               0: (0, 0) -> 1
   *                                       1: (0, 1) -> 2
   * 1: array 
   * 2: array         0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3: array                              2: (3, 0) -> 4
   * 4: cons                               3: (4, 1) -> 0
   *                                       4: (4, 0) -> 4
   * 
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   * 5: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set(4));
  REQUIRE(Parents(sim, 1) == Set(0));
  REQUIRE(Parents(sim, 2) == Set(0));
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set(3, 4));

  REQUIRE(sim.numRoot() == 1);
  REQUIRE(sim.numBulk() == 4);
  REQUIRE(sim.numChildren() == 5);
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 6);
  REQUIRE(sim.checkSanity());


  sim.removeEdge(std::make_pair(0, 0));
  /*
   * containers       freeChildren         children
   * 0: cons          0: (0, 0) -> null    0: (0, 1) -> 2
   * 2: array         1: (2, 0) -> null
   *                  2: (2, 1) -> null
   *                  3: (2, 2) -> null
   * 3: array                              1: (3, 0) -> 4
   * 4: cons                               2: (4, 1) -> 0
   *                                       3: (4, 0) -> 4
   * 
   * edges:
   * 0: (0, 1) -> 2
   * 1: (3, 0) -> 4
   * 2: (4, 1) -> 0
   * 3: (4, 0) -> 4
   * 4: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set(4));
  REQUIRE(Parents(sim, 1) == Set());
  REQUIRE(Parents(sim, 2) == Set(0));
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set(3, 4));

  REQUIRE(sim.numRoot() == 1);
  REQUIRE(sim.numBulk() == 3);
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numChildren() == 4);
  REQUIRE(sim.numEdges() == 5);
  REQUIRE(sim.checkSanity());

  sim.removeEdge(std::make_pair(4, 1));
  /*
   * containers       freeChildren         children
   * 3: array                              1: (3, 0) -> 4
   * 4: cons          2: (4, 1) -> null    3: (4, 0) -> 4                 
   * 
   * edges:
   * 0: (3, 0) -> 4
   * 1: (4, 0) -> 4
   * 2: R -> 3
   * 
   */
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(Parents(sim, 1) == Set());
  REQUIRE(Parents(sim, 2) == Set());
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set(3, 4));

  REQUIRE(sim.numRoot() == 1);
  REQUIRE(sim.numBulk() == 1);
  REQUIRE(sim.numChildren() == 2);
  REQUIRE(sim.numFreeChildren() == 1);
  REQUIRE(sim.numEdges() == 3);
  REQUIRE(sim.checkSanity());

  sim.removeEdge(std::make_pair(GcSim::ROOT(), 3));
  REQUIRE(Parents(sim, 0) == Set());
  REQUIRE(Parents(sim, 1) == Set());
  REQUIRE(Parents(sim, 2) == Set());
  REQUIRE(Parents(sim, 3) == Set());
  REQUIRE(Parents(sim, 4) == Set());

  REQUIRE(sim.numRoot() == 0);
  REQUIRE(sim.numBulk() == 0);
  REQUIRE(sim.numChildren() == 0);
  REQUIRE(sim.numFreeChildren() == 0);
  REQUIRE(sim.numEdges() == 0);
  REQUIRE(sim.checkSanity());
}


static std::set<std::size_t> Parents(const GcSim & gcSim, std::size_t index)
{
  std::set<std::size_t> ret;
  auto graph = gcSim.getGraph();
  auto node = graph->findNode(gcSim.getCell(index));
  if(node)
  {
    auto parents = node->getParents();
    for(const Cell & cell : parents)
    {
      ret.insert(gcSim.getCellIndex(cell));
    }
  }
  return ret;
}
