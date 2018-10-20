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

//static std::set<std::size_t> Parents(const GcSim & gcSim, std::size_t index);
static std::unordered_set<Lisp::Cell> Set()
{
  return std::unordered_set<Lisp::Cell>();
}

template<typename... A>
static std::unordered_set<Lisp::Cell> Set(const Lisp::Cell & cell, A... rest) {
  std::unordered_set<Lisp::Cell> s = Set(rest...);
  s.insert(cell);
  return s;
}

inline bool checkNodes(const GcSim & sim,
                       const std::unordered_set<Lisp::Cell> & rootNodes,
                       const std::unordered_set<Lisp::Cell> & bulkNodes)
{
  bool ret = true;
  std::unordered_set<Lisp::Cell> _rootNodes;
  std::unordered_set<Lisp::Cell> _bulkNodes;
  std::unordered_set<Lisp::Cell> _allNodes;
  std::unordered_set<Lisp::Cell> _all(rootNodes.begin(), rootNodes.end());
  _all.insert(bulkNodes.begin(), bulkNodes.end());
  for(std::size_t i = 0; i < sim.numRoot(); i++)
  {
    _rootNodes.insert(sim.getNthRoot(i));
  }
  for(std::size_t i = 0; i < sim.numBulk(); i++)
  {
    _bulkNodes.insert(sim.getNthBulk(i));
  }
  for(std::size_t i = 0; i < sim.numTotal(); i++)
  {
    _allNodes.insert(sim.getNthNode(i));
  }
  CHECK(sim.numRoot() == rootNodes.size());
  ret &= (sim.numRoot() == rootNodes.size());
  CHECK(sim.numBulk() == bulkNodes.size());
  ret &= sim.numBulk() == bulkNodes.size();
  CHECK(sim.numTotal() == (bulkNodes.size() + rootNodes.size()));
  ret &= sim.numTotal() == (bulkNodes.size() + rootNodes.size());
  CHECK(rootNodes == _rootNodes);
  ret &= rootNodes == _rootNodes;
  CHECK(bulkNodes == _bulkNodes);
  ret &= bulkNodes == _bulkNodes;
  CHECK(_all == _allNodes);
  ret &= _all == _allNodes;
  return ret;
}

TEST_CASE("gc_sim_cycle", "[GcSim]")
{
  GcSim sim;
  sim.disableCollector();
  REQUIRE(checkNodes(sim, Set(), Set()));
  REQUIRE(sim.numFreeChildren() == 0);
  REQUIRE(sim.numEdges() == 0);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  Cell cell0(sim.addRoot(2));
  /*
   * containers       freeChildren
   * 0 (R): cons      0: (0, 0) -> null
   *                  1: (0, 1) -> null
   * edges:
   * 
   */
  REQUIRE(checkNodes(sim, Set(cell0), Set()));
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.numFreeChildren() == 2);
  REQUIRE(sim.numEdges() == 0);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  Cell cell1(sim.addBulk(cell0, 0, 0));
  //REQUIRE(sim.addBulk(std::make_pair(0, 0), 0) == 1);
  /*
   * containers       freeChildren
   * 0 (R): cons      0: (0, 1) -> null
   * 1: array         
   *
   * edges:
   * 0: (0, 0) -> 1
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(checkNodes(sim, Set(cell0), Set(cell1)));
  REQUIRE(sim.numFreeChildren() == 1);
  REQUIRE(sim.numEdges() == 1);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  Cell cell2(sim.addBulk(cell0, 1, 3));
    /*
   * containers       freeChildren
   * 0 (R): cons(0,1)
   *
   * 1: array()
   * 2: array(0,1,2)  0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(checkNodes(sim, Set(cell0), Set(cell1, cell2)));
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 2);

  ///////////////////////////////////////////
  Cell cell3(sim.addRoot(1));
  /*
   * containers       freeChildren
   * 0 (R): cons
   *
   * 1: array 
   * 2: array         0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3 (R): array     3: (3, 0) -> null
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   */
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(checkNodes(sim,
                     Set(cell0, cell3),
                     Set(cell1, cell2)));
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numEdges() == 2);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  Cell cell4(sim.addBulk(cell3, 0, 2));
  /*
   * containers       freeChildren         children
   * 0 (R): cons
   *
   * 1: array 
   * 2: array         0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3 (R): array
   * 4: cons          3: (4, 0) -> null
   *                  4: (4, 1) -> null
   * 
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   */
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3));
  REQUIRE(checkNodes(sim,
                     Set(cell0, cell3),
                     Set(cell1, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 5);
  REQUIRE(sim.numEdges() == 3);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  sim.addEdge(cell4, 1, cell0);
  /*
   * containers       freeChildren
   * 0 (R): cons
   *
   * 1: array 
   * 2: arra          0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3 (R): array
   * 4: cons          3: (4, 0) -> null
   * 
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   */
  REQUIRE(sim.getParents(cell0) == Set(cell4));
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3));
  REQUIRE(checkNodes(sim,
                     Set(cell0, cell3),
                     Set(cell1, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numEdges() == 4);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  sim.addEdge(cell4, 0, cell4);
  /*
   * containers       freeChildren
   * 0 (R): cons
   * 1: array 
   * 2: array         0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3 (R): array
   * 4: cons
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set(cell4));
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3, cell4));
  REQUIRE(checkNodes(sim,
                     Set(cell0, cell3),
                     Set(cell1, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 5);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  REQUIRE(cell0.isRoot());
  sim.unroot(cell0);
  REQUIRE_FALSE(cell0.isRoot());
  /*
   * containers       freeChildren
   * 0: cons
   * 1: array 
   * 2: array         0: (2, 0) -> null
   *                  1: (2, 1) -> null
   *                  2: (2, 2) -> null
   * 3 (R): array
   * 4: cons
   *
   * edges:
   * 0: (0, 0) -> 1
   * 1: (0, 1) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   */
  REQUIRE(sim.getParents(cell0) == Set(cell4));
  REQUIRE(sim.getParents(cell1) == Set(cell0));
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3, cell4));
  REQUIRE(checkNodes(sim,
                     Set(cell3),
                     Set(cell0, cell1, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 5);
  REQUIRE(sim.getNthRoot(0) == cell3);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  sim.removeEdge(cell0, 0);
  /*
   * containers       freeChildren         children
   * 0: cons          0: (0, 0) -> null    0: (0, 1) -> 2
   * 2: array         1: (2, 0) -> null
   *                  2: (2, 1) -> null
   *                  3: (2, 2) -> null
   * 3 (R): array
   * 4: cons
   *
   * edges:
   * 0: (0, 1) -> 2
   * 1: (3, 0) -> 4
   * 2: (4, 1) -> 0
   * 3: (4, 0) -> 4
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set(cell4));
  REQUIRE(sim.getParents(cell1) == Set());
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3, cell4));
  REQUIRE(checkNodes(sim,
                     Set(cell3),
                     Set(cell0, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numEdges() == 4);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  sim.addEdge(cell0, 0, cell2);
  /*
   * containers       freeChildren         children
   * 0: cons          1: (2, 0) -> null    0: (0, 1) -> 2
   * 2: array         2: (2, 1) -> null    0: (0, 0) -> 2
   *                  3: (2, 2) -> null
   *
   * 3 (R): array
   * 4: cons
   *
   * edges:
   * 0: (0, 1) -> 2
   * 1: (0, 0) -> 2
   * 2: (3, 0) -> 4
   * 3: (4, 1) -> 0
   * 4: (4, 0) -> 4
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set(cell4));
  REQUIRE(sim.getParents(cell1) == Set());
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3, cell4));
  REQUIRE(checkNodes(sim,
                     Set(cell3),
                     Set(cell0, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 3);
  REQUIRE(sim.numEdges() == 5);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  sim.removeEdge(cell0, 1);
  /*
   * containers       freeChildren         children
   * 0: cons          0: (0, 1) -> null    0: (0, 0) -> 2
   * 2: array         1: (2, 0) -> null
   *                  2: (2, 1) -> null
   *                  3: (2, 2) -> null
   * 3 (R): array
   * 4: cons
   *
   * edges:
   * 0: (0, 1) -> 2
   * 1: (3, 0) -> 4
   * 2: (4, 1) -> 0
   * 3: (4, 0) -> 4
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set(cell4));
  REQUIRE(sim.getParents(cell1) == Set());
  REQUIRE(sim.getParents(cell2) == Set(cell0));
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3, cell4));
  REQUIRE(checkNodes(sim,
                     Set(cell3),
                     Set(cell0, cell2, cell4)));
  REQUIRE(sim.numFreeChildren() == 4);
  REQUIRE(sim.numEdges() == 4);
  REQUIRE(sim.checkSanity());
  
  ///////////////////////////////////////////
  sim.removeEdge(cell4, 1);
  /*
   * containers       freeChildren
   * 3 (R): array     2: (4, 1) -> null
   * 4: cons          
   * 
   * edges:
   * 0: (3, 0) -> 4
   * 1: (4, 0) -> 4
   * 
   */
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.getParents(cell1) == Set());
  REQUIRE(sim.getParents(cell2) == Set());
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set(cell3, cell4));
  REQUIRE(checkNodes(sim,
                     Set(cell3),
                     Set(cell4)));
  REQUIRE(sim.numFreeChildren() == 1);
  REQUIRE(sim.numEdges() == 2);
  REQUIRE(sim.checkSanity());

  ///////////////////////////////////////////
  sim.unroot(cell3);
  REQUIRE(sim.getParents(cell0) == Set());
  REQUIRE(sim.getParents(cell1) == Set());
  REQUIRE(sim.getParents(cell2) == Set());
  REQUIRE(sim.getParents(cell3) == Set());
  REQUIRE(sim.getParents(cell4) == Set());
  REQUIRE(checkNodes(sim,
                     Set(),
                     Set()));
  REQUIRE(sim.numFreeChildren() == 0);
  REQUIRE(sim.numEdges() == 0);
  REQUIRE(sim.checkSanity());
}
