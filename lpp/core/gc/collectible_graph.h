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
#include <lpp/core/cell.h>

namespace Lisp
{
  class Collectible;
  class GarbageCollector;
  class CollectibleNode;
  class CollectibleEdge;

  class CollectibleGraph
  {
  public:
    using SharedNode = std::shared_ptr<CollectibleNode>;

    CollectibleGraph(const GarbageCollector & collector);
    std::shared_ptr<CollectibleNode> findNode(const Cell & cell) const;
    std::shared_ptr<CollectibleNode> getNode(std::size_t index) const;
    std::shared_ptr<CollectibleNode> getBulkNode(std::size_t index) const;
    std::shared_ptr<CollectibleNode> getRootNode(std::size_t index) const;
    std::shared_ptr<CollectibleEdge> getEdge(std::size_t index) const;
    void forEachNode(std::function<void(const CollectibleNode & node)> func) const;
    void forEachNode(std::function<void(const SharedNode & node)> func) const;
    inline std::size_t numNodes() const;
    inline std::size_t numEdges() const;
  private:
    std::unordered_map<Cell, SharedNode> nodes;
    std::vector<std::shared_ptr<CollectibleEdge>> edges;
    std::vector<SharedNode> rootNodes;
    std::vector<SharedNode> bulkNodes;
    std::vector<SharedNode> allNodes;
  };
}

inline std::shared_ptr<Lisp::CollectibleNode> Lisp::CollectibleGraph::getNode(std::size_t index) const
{
  return allNodes.at(index);
}

inline std::shared_ptr<Lisp::CollectibleNode> Lisp::CollectibleGraph::getBulkNode(std::size_t index) const
{
  return bulkNodes.at(index);
}

inline std::shared_ptr<Lisp::CollectibleNode> Lisp::CollectibleGraph::getRootNode(std::size_t index) const
{
  return rootNodes.at(index);
}


inline std::size_t Lisp::CollectibleGraph::numNodes() const
{
  return nodes.size();
}

inline std::size_t Lisp::CollectibleGraph::numEdges() const
{
  return edges.size();
}

