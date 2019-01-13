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
#pragma once
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/simul/collectible_graph.h>
#include <lpp/simul/collectible_edge.h>
#include <lpp/simul/collectible_node.h>

#include <set>
#include <map>
#include <unordered_map>
#include <limits>
#include <list>
#include <utility>
#include <memory>

namespace Lisp
{
  class GcSim
  {
  public:
    GcSim();

    /**
     * Number of root objects (directly reachable)
     */
    inline std::size_t numRoot() const;

    /**
     * Number of bulk objects (reachable from root) 
     */
    inline std::size_t numBulk() const;

    /**
     * Total number of objects.
     * numTotal = numRoot + numBulk
     */
    inline std::size_t numTotal() const;

    /**
     * Number of allocated objects.
     * The (numAllocated - numTotal) is the number
     * of unreachable objects. 
     */
    inline std::size_t numAllocated() const;

    /**
     * Number of leave nodes of the reachibility graph.
     * That is the number of child objects that are not a Collectible
     */
    inline std::size_t numLeaves() const;

    /**
     * Number of edges of the dependency graph.
     */
    inline std::size_t numEdges() const;

    /** 
     * Void Conses that have been marked as unreachable.
     */
    inline std::size_t numVoid() const;

    inline std::size_t numDisposed() const;

    /** 
     * Number of full garbage collector cycles
     */
    inline std::size_t numCycles() const;

    inline void setGarbageSteps(std::size_t steps);
    inline void setRecycleSteps(std::size_t steps);
    
    // n in [0, numRoot())
    inline const Cell & getNthRoot(std::size_t n) const;

    // n in [0, numBulk())
    inline const Cell & getNthBulk(std::size_t n) const;

    // n in [0, numTotal())
    inline const Cell & getNthNode(std::size_t n) const;

    inline std::unordered_set<Lisp::Cell> getParents(const Lisp::Cell & cell) const;
    inline std::vector<std::pair<Lisp::Cell, std::size_t>> getParentIndices(const Lisp::Cell & cell) const;

    inline const Lisp::Cell & addRoot(std::size_t nchildren);
    inline const Lisp::Cell & addBulk(std::size_t freeChildIndex, std::size_t ncells);
    inline const Lisp::Cell & addBulk(Lisp::Cell & cell, std::size_t index, std::size_t ncells);
    inline void addEdge(std::size_t freeChildIndex, std::size_t nodeIndex);
    inline void addEdge(Cell & cell, std::size_t index, const Cell & child);
    inline void removeEdge(std::size_t index);
    inline void removeEdge(Cell & cell, std::size_t index);
    inline void unroot(std::size_t index);
    inline void unroot(const Cell & cell);

    inline bool checkSanity() const;
    inline void disableCollector();
    inline void disableRecycling();
    inline void enableCollector();
    inline void enableRecycling();

  private:
    inline const CollectibleGraph & getGraph() const;
    inline Cell createNode(std::size_t ncells);
    inline void setChild(Cell & cell, std::size_t index, const Cell & child);
    GarbageCollector gc;
    mutable std::shared_ptr<CollectibleGraph> graph;
    std::vector<std::shared_ptr<Object>> root;
  };
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
///////////////////////////////////////////////////////////////////////////////
inline Lisp::GcSim::GcSim()
{}
      
inline std::size_t Lisp::GcSim::numRoot() const
{
  return root.size();
}

inline std::size_t Lisp::GcSim::numBulk() const
{
  return getGraph().numNodes() - root.size();
}

inline std::size_t Lisp::GcSim::numTotal() const
{
  return getGraph().numNodes();
}

inline std::size_t Lisp::GcSim::numAllocated() const
{
  return gc.numCollectible();
}

inline std::size_t Lisp::GcSim::numLeaves() const
{
  return getGraph().numLeaves();
}

inline std::size_t Lisp::GcSim::numEdges() const
{
  return getGraph().numEdges();
}

inline std::size_t Lisp::GcSim::numVoid() const
{
  return gc.numVoidCollectible();
}

inline std::size_t Lisp::GcSim::numDisposed() const
{
  return gc.numDisposedCollectible();
}

inline std::size_t Lisp::GcSim::numCycles() const
{
  return gc.getCycles();
}

inline void Lisp::GcSim::setGarbageSteps(std::size_t steps)
{
  gc.setGarbageSteps(steps);
}

inline void Lisp::GcSim::setRecycleSteps(std::size_t steps)
{
  gc.setRecycleSteps(steps);
}

inline const Lisp::Cell & Lisp::GcSim::getNthRoot(std::size_t n) const
{
  return getGraph().getRootNode(n)->getCell();
}

inline const Lisp::Cell & Lisp::GcSim::getNthBulk(std::size_t n) const
{
  return getGraph().getBulkNode(n)->getCell();
}

inline const Lisp::Cell & Lisp::GcSim::getNthNode(std::size_t n) const
{
  return getGraph().getNode(n)->getCell();
}

inline std::unordered_set<Lisp::Cell> Lisp::GcSim::getParents(const Lisp::Cell & cell) const
{
  auto node = getGraph().findNode(cell);
  if(node)
  {
    std::unordered_set<Cell> ret;
    for(auto & p : node->getParents())
    {
      ret.insert(p.first);
    }
    return ret;
  }
  else
  {
    return std::unordered_set<Cell>();
  }
}

inline std::vector<std::pair<Lisp::Cell, std::size_t>> Lisp::GcSim::getParentIndices(const Lisp::Cell & cell) const
{
  auto node = getGraph().findNode(cell);
  if(node)
  {
    return node->getParents();
  }
  else
  {
    return std::vector<std::pair<Lisp::Cell, std::size_t>>();
  }
}

inline const Lisp::Cell & Lisp::GcSim::addRoot(std::size_t nchildren)
{
  std::shared_ptr<Object> obj;
  root.push_back(std::shared_ptr<Object>());
  if(nchildren == 2)
  {
    root.back().reset(new Object(gc.makeRoot<Cons>(Lisp::nil, Lisp::nil)));
  }
  else
  {
    auto arr = gc.makeRoot<Array>();
    for(std::size_t i = 0; i < nchildren; i++)
    {
      arr->append(Lisp::nil);
    }
    root.back().reset(new Object(arr));
  }
  graph.reset();
  return *(root.back().get());
}

inline const Lisp::Cell & Lisp::GcSim::addBulk(std::size_t leafIndex, std::size_t nchildren)
{
  const CollectibleGraph & gr(getGraph());
  assert(leafIndex < gr.numLeaves());
  auto p = gr.getLeaf(leafIndex);
  auto node = p.first.lock();
  assert(bool(node));
  Cell cell(node->getCell());
  return addBulk(cell, p.second, nchildren);
}

inline const Lisp::Cell & Lisp::GcSim::addBulk(Lisp::Cell & cell, std::size_t index, std::size_t nchildren)
{
  graph.reset();
  if(cell.isA<Cons>())
  {
    auto cons = cell.as<Cons>();
    if(index == 0)
    {
      assert(cons->getCarCell().isA<Lisp::Nil>());
      cons->setCar(createNode(nchildren));
      return cons->getCarCell();
    }
    else if(index == 1)
    {
      assert(cons->getCdrCell().isA<Lisp::Nil>());
      cons->setCdr(createNode(nchildren));
      return cons->getCdrCell();
    }
    else
    {
      throw std::out_of_range("invalid index (>1) for cons");
    }
  }
  else if(cell.isA<Array>())
  {
    auto arr = cell.as<Array>();
    if(index < arr->size())
    {
      assert(arr->atCell(index).isA<Lisp::Nil>());
      arr->set(index, createNode(nchildren));
      return arr->atCell(index);
    }
    else
    {
      throw std::out_of_range("index of array out of range");
    }
  }
}

inline void Lisp::GcSim::addEdge(std::size_t leafIndex, std::size_t nodeIndex)
{
  const CollectibleGraph & gr(getGraph());
  assert(leafIndex < gr.numLeaves());
  auto p = gr.getLeaf(leafIndex);
  auto node = p.first.lock();
  assert(bool(node));
  Cell cell(node->getCell());
  addEdge(cell, p.second, getNthNode(nodeIndex));
}

inline void Lisp::GcSim::addEdge(Cell & cell, std::size_t index, const Cell & child)
{
  auto c = getGraph().findNode(cell);
  setChild(cell, index, child);
  graph.reset();
}

inline void Lisp::GcSim::removeEdge(std::size_t edge)
{
  auto p = getGraph().getEdge(edge);
  Cell cell(p->getParent());
  removeEdge(cell, p->getIndex());
}

inline void Lisp::GcSim::removeEdge(Cell & cell, std::size_t index)
{
  auto node = getGraph().findNode(cell);
  setChild(cell, index, Lisp::nil);
  graph.reset();
}

inline const Lisp::CollectibleGraph & Lisp::GcSim::getGraph() const
{
  if(!graph)
  {
    graph.reset(new CollectibleGraph(gc));
  }
  return *graph;
}

inline Lisp::Cell Lisp::GcSim::createNode(std::size_t nchildren)
{
  if(nchildren == 2)
  {
    return gc.make<Cons>(Lisp::nil, Lisp::nil);
  }
  else
  {
    Array * array = gc.make<Array>();
    for(std::size_t i = 0; i < nchildren; i++)
    {
      array->append(Lisp::nil);
    }
    return array;
  }
}

inline void Lisp::GcSim::setChild(Cell & cell, std::size_t index, const Cell & child)
{
  if(cell.isA<Cons>())
  {
    auto cons = cell.as<Cons>();
    if(index == 0)
    {
      cons->setCar(child);
    }
    else if(index == 1)
    {
      cons->setCdr(child);
    }
    else
    {
      throw std::out_of_range("invalid index (>1) for cons");
    }
  }
  else if(cell.isA<Array>())
  {
    auto array = cell.as<Array>();
    if(index < array->size())
    {
      array->set(index, child);
    }
    else
    {
      throw std::out_of_range("index of array out of range");
    }
  }
}

inline void Lisp::GcSim::unroot(std::size_t index)
{
  assert(index < root.size());
  root[index].swap(root.back());
  root.pop_back();
  graph.reset();
}

inline void Lisp::GcSim::unroot(const Cell & cell)
{
  std::equal_to<::Lisp::Cell> eq;
  for(std::size_t i = 0; i < root.size(); i++)
  {
    if(eq(*root[i], cell))
    {
      unroot(i);
      return;
    }
  }
}

inline bool Lisp::GcSim::checkSanity() const
{
  bool ret = gc.checkSanity();
  return ret;
}

inline void Lisp::GcSim::disableCollector()
{
  gc.disableCollector();
}

inline void Lisp::GcSim::disableRecycling()
{
  gc.disableRecycling();
}

inline void Lisp::GcSim::enableCollector()
{
  gc.enableCollector();
}

inline void Lisp::GcSim::enableRecycling()
{
  gc.enableRecycling();
}
