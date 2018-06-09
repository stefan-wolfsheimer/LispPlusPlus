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
#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <lpp/core/lisp_cell.h>

namespace Lisp
{
  class Collectible;
  class GarbageCollector;
  class CollectibleNode;
  class CollectibleEdge;

  class CollectibleGraph
  {
  public:
    CollectibleGraph(const GarbageCollector & collector);
    std::shared_ptr<CollectibleEdge> findEdge(const Cell & parent,
                                              const Cell & child) const;
    std::shared_ptr<CollectibleNode> findNode(const Cell & cell) const;
    std::shared_ptr<CollectibleNode> getNode(std::size_t index) const;
    std::shared_ptr<CollectibleEdge> getEdge(std::size_t index) const;
    inline std::size_t numNodes() const;
    inline std::size_t numEdges() const;
  private:
    using SharedNode = std::shared_ptr<CollectibleNode>;
    using Pair = std::pair<Cell, Cell>;
    struct PairHash
    {
      std::size_t operator()(const Pair & p) const
      {
        return std::hash<Cell>()(p.first) ^ std::hash<Cell>()(p.second);
      }
    };
    struct PairEq
    {
      bool operator()(const Pair & p1, const Pair & p2) const
      {
        return std::equal_to<Cell>()(p1.first, p2.first) &&
               std::equal_to<Cell>()(p1.second, p2.second);
      }
    };
    std::unordered_map<Cell, SharedNode> nodes;
    std::unordered_map<Pair, std::shared_ptr<CollectibleEdge>, PairHash, PairEq> edges;
  };
}

inline std::size_t Lisp::CollectibleGraph::numNodes() const
{
  return nodes.size();
}

inline std::size_t Lisp::CollectibleGraph::numEdges() const
{
  return edges.size();
}

