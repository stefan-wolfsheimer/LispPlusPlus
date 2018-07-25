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
#include <lpp/core/gc/collectible_graph.h>
#include <lpp/core/gc/collectible_edge.h>
#include <lpp/core/gc/collectible_node.h>

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
    inline std::size_t numRoot() const;
    inline std::size_t numBulk() const;
    inline std::size_t numTotal() const;
    inline std::size_t numChildren() const;
    inline std::size_t numFreeChildren() const;
    inline std::size_t numEdges() const;
    inline const Cell & getCell(std::size_t index) const;
    inline std::size_t getCellIndex(const Cell & cell) const;

    inline std::size_t addRoot(std::size_t nchildren);
    inline std::size_t addBulk(std::size_t freeChildIndex, std::size_t ncells);
    inline std::size_t addBulk(std::pair<std::size_t, std::size_t> freeChild, std::size_t ncells);

    
    /////////////////////

    inline void addEdge(std::size_t freeChildIndex, std::size_t nodeIndex);
    inline void addEdge(std::pair<std::size_t, std::size_t> freeChild, std::size_t nodeIndex);
    inline void removeEdge(std::size_t edge);
    inline void removeEdge(std::pair<std::size_t, std::size_t> edge);
    inline bool checkSanity() const;
    inline void disableCollector();
    inline void disableRecycling();
    inline void enableCollector();
    inline void enableRecycling();

    static std::size_t ROOT() { return std::numeric_limits<std::size_t>::max(); }

    std::shared_ptr<Lisp::CollectibleGraph> getGraph() const;
    inline void refresh();
  private:

    inline void refreshCell(std::size_t nodeIndex,
                            const Cell & cell);
    
    GarbageCollector gc;
    mutable std::shared_ptr<Lisp::CollectibleGraph> graph;

    // all objects in the root set
    std::vector<std::shared_ptr<Object>> root;

    // all reachable cells with ids
    std::unordered_map<Cell, std::size_t> allInv;
    std::unordered_map<std::size_t, Cell> all;
    std::size_t nextCellId;

    // set of cells of nodes that point to another object
    // first -> index of the node in vector all
    // second -> index of the cell of the node (for conses 0: car, 1: cdr)
    std::set<std::pair<std::size_t, std::size_t>> children;

    // set of cell of nodes that are nil
    // first -> index of the node in vector all
    // second -> index of the cell of the node (for conses 0: car, 1: cdr)
    std::set<std::pair<std::size_t, std::size_t>> freeChildren;

    // all edges
    // first.first: index of the parent in map all
    // first.second: index of the cell
    // second:       index of the child in map all
    std::map<std::pair<std::size_t, std::size_t>, std::size_t> edges;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
inline Lisp::GcSim::GcSim()
{
  nextCellId = 0;
}
      
inline std::size_t Lisp::GcSim::numRoot() const
{
  return root.size();
}

inline std::size_t Lisp::GcSim::numBulk() const
{
  return all.size() - root.size();
}

inline std::size_t Lisp::GcSim::numTotal() const
{
  return all.size();
}

inline std::size_t Lisp::GcSim::numFreeChildren() const
{
  return freeChildren.size();
}

inline std::size_t Lisp::GcSim::numChildren() const
{
  return children.size();
}

inline std::size_t Lisp::GcSim::numEdges() const
{
  return edges.size();
}

inline const Lisp::Cell & Lisp::GcSim::getCell(std::size_t index) const
{
  auto itr = all.find(index);
  if(itr != all.end())
  {
    return itr->second;
  }
  return Lisp::nil;
}

inline std::size_t Lisp::GcSim::getCellIndex(const Lisp::Cell & cell) const
{
  auto itr = allInv.find(cell);
  if(itr != allInv.end())
  {
    return itr->second;
  }
  return ROOT();
}

inline std::size_t Lisp::GcSim::addRoot(std::size_t nchildren)
{
  graph.reset();
  std::shared_ptr<Object> obj;
  std::size_t cellId;
  if(nchildren == 2)
  {
    obj = std::make_shared<Object>(gc.makeRootCons(Lisp::nil, Lisp::nil));
  }
  else
  {
    auto arr = gc.makeRoot<Array>();
    //@todo better solution for appending
    for(std::size_t i = 0; i < nchildren; i++)
    {
      arr->append(Lisp::nil);
    }
    obj = std::make_shared<Object>(arr);
  }
  cellId = nextCellId++;
  allInv.insert(std::make_pair(*obj, cellId));
  all.insert(std::make_pair(cellId, *obj));
  root.push_back(obj);
  for(std::size_t i = 0; i < nchildren; i++)
  {
    freeChildren.insert(std::make_pair(cellId, i));
  }
  edges.insert(std::make_pair(std::make_pair(ROOT(), cellId), cellId));
  return cellId;
}

inline std::size_t Lisp::GcSim::addBulk(std::size_t freeChildIndex, std::size_t nchildren)
{
  graph.reset();
  assert(freeChildIndex < freeChildren.size());
  auto itr = freeChildren.begin();
  while(freeChildIndex > 0)
  {
    freeChildIndex--;
    itr++;
  }
  return addBulk(*itr, nchildren);
}


inline std::size_t Lisp::GcSim::addBulk(std::pair<std::size_t, std::size_t> pair, std::size_t nchildren)
{
  graph.reset();
  const Cell * tobeadded = nullptr;
  assert(all.find(pair.first) != all.end());
  if(all[pair.first].isA<Cons>())
  {
    auto cons = all[pair.first].as<Cons>();
    if(pair.second == 0)
    {
      cons->setCar(nchildren == 2 ?
                   Cell(gc.makeCons(Lisp::nil, Lisp::nil)) :
                   Cell(gc.make<Array>()));
      tobeadded = &cons->getCarCell();
    }
    else if(pair.second == 1)
    {
      cons->setCdr(nchildren == 2 ?
                   Cell(gc.makeCons(Lisp::nil, Lisp::nil)) :
                   Cell(gc.make<Array>()));
      tobeadded = &cons->getCdrCell();
    }
    else
    {
      throw std::out_of_range("invalid index (>1) for cons");
    }
  }
  else if(all[pair.first].isA<Array>())
  {
    auto arr = all[pair.first].as<Array>();
    if(pair.second < arr->size())
    {
      arr->set(pair.second, (nchildren == 2 ?
                             Cell(gc.makeCons(Lisp::nil, Lisp::nil)) :
                             Cell(gc.make<Array>())));
      tobeadded = &arr->atCell(pair.second);
    }
    else
    {
      throw std::out_of_range("index of array out of range");
    }
  }
  else
  {
    throw std::logic_error(std::string("unsupported type '") + all[pair.first].getTypeName() + "'");
  }
  if(nchildren != 2)
  {
    //@todo better solution for appending
    for(std::size_t i = 0; i < nchildren; i++)
    {
      tobeadded->as<Array>()->append(Lisp::nil);
    }
  }
  freeChildren.erase(pair);
  children.insert(pair);
  std::size_t cellId = nextCellId++;
  edges.insert(std::make_pair(pair, cellId));
  for(std::size_t i = 0; i < nchildren; i++)
  {
    freeChildren.insert(std::make_pair(cellId, i));
  }
  all[cellId] = *tobeadded;
  allInv[*tobeadded] = cellId;
  return cellId;
}

inline void Lisp::GcSim::addEdge(std::size_t freeChildIndex, std::size_t nodeIndex)
{
  graph.reset();
  assert(freeChildIndex < freeChildren.size());
  auto itr = freeChildren.begin();
  while(freeChildIndex > 0)
  {
    freeChildIndex--;
    itr++;
  }
  addEdge(*itr, nodeIndex);
}

inline void Lisp::GcSim::addEdge(std::pair<std::size_t, std::size_t> pair, std::size_t nodeIndex)
{
  graph.reset();
  assert(nodeIndex < all.size());
  assert(all.find(pair.first) != all.end());
  if(all[pair.first].isA<Cons>())
  {
    auto cons = all[pair.first].as<Cons>();
    if(pair.second == 0)
    {
      cons->setCar(all[nodeIndex]);
    }
    else if(pair.second == 1)
    {
      cons->setCdr(all[nodeIndex]);
    }
    else
    {
      throw std::out_of_range("invalid index (>1) for cons");
    }
  }
  else if(all[pair.first].isA<Array>())
  {
    auto arr = all[pair.first].as<Array>();
    if(pair.second < arr->size())
    {
      arr->set(pair.second, Cell(all[nodeIndex]));
    }
    else
    {
      throw std::out_of_range("index of array out of range");
    }
  }
  freeChildren.erase(pair);
  children.insert(pair);
  edges.insert(std::make_pair(pair, nodeIndex));
}

inline void Lisp::GcSim::refreshCell(std::size_t nodeIndex,
                                     const Cell & cell)
{
  std::size_t childIndex = 0;
  cell.forEachChild([this, &childIndex, nodeIndex](const Cell & child) {
      if(child.isA<Nil>())
      {
        freeChildren.insert(std::make_pair(nodeIndex, childIndex));
      }
      else
      {
        auto p = std::make_pair(nodeIndex, childIndex);
        auto itr = allInv.find(child);
        edges.insert(std::make_pair(p, itr->second));
        children.insert(p);
      }
      childIndex++;
    });
}

inline void Lisp::GcSim::refresh()
{
  std::unordered_map<Cell, std::size_t> tmpInv;
  std::unordered_map<std::size_t, Cell> tmp;
  children.clear();
  freeChildren.clear();
  edges.clear();
  gc.forEachReachable([this, &tmpInv, &tmp](const Cell & c){
      auto itr = allInv.find(c);
      assert(itr != allInv.end());
      tmpInv.insert(*itr);
      tmp.insert(std::make_pair(itr->second, itr->first));
    });
  allInv.swap(tmpInv);
  all.swap(tmp);
  gc.forEachReachable([this](const Cell & c){
      auto itr = allInv.find(c);
      refreshCell(itr->second, itr->first);
    });
  for(auto r : root)
  {
    auto itr = allInv.find(*r);
    edges.insert(std::make_pair(std::make_pair(ROOT(), itr->second), itr->second));
  }
}

//////////////////////////////////////////
inline void Lisp::GcSim::removeEdge(std::size_t edge)
{
  graph.reset();
  assert(edge < edges.size());
  auto itr = edges.begin();
  while(edge > 0)
  {
    edge--;
    itr++;
  }
  removeEdge(itr->first);
}

inline void Lisp::GcSim::removeEdge(std::pair<std::size_t, std::size_t> edge)
{
  graph.reset();
  auto itr = edges.find(edge);
  if(itr != edges.end())
  {
    if(itr->first.first == ROOT())
    {
      Cell & cell(all[itr->second]);
      for(std::size_t i = 0; i < root.size(); i++)
      {
        if(*root[i] == cell)
        {
          root[i] = root.back();
          root.pop_back();
          break;
        }
      }
      edges.erase(itr);
    }
    else
    {
      auto pair = itr->first;
      auto nodeIndex = itr->second;
      if(all[pair.first].isA<Cons>())
      {
        auto cons = all[pair.first].as<Cons>();
        if(pair.second == 0)
        {
          cons->setCar(Lisp::nil);
        }
        else if(pair.second == 1)
        {
          cons->setCdr(Lisp::nil);
        }
      }
      edges.erase(itr);
    }
    refresh();
  }
}

inline bool Lisp::GcSim::checkSanity() const
{
  bool ret = gc.checkSanity();
  ret &= (numRoot() + numBulk() == numTotal());
  for(auto p : all)
  {
    auto itr = allInv.find(p.second);
    if(itr == allInv.end())
    {
      ret = false;
    }
    else
    {
      ret &= (p.first == itr->second && p.second == itr->first);
    }
  }
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

std::shared_ptr<Lisp::CollectibleGraph>  Lisp::GcSim::getGraph() const
{
  if(!graph)
  {
    graph.reset(new Lisp::CollectibleGraph(gc));
  }
  return graph;
}
