/******************************************************************************
Copyright (c) 2017, Stefan Wolfsheimer

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
#include <stdexcept>
#include <string>
#include <lpp/simul/collectible_graph.h>
#include <lpp/simul/collectible_node.h>
#include <lpp/simul/collectible_edge.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/collectible.h>

using CollectibleGraph = Lisp::CollectibleGraph;
using CollectibleEdge =  Lisp::CollectibleEdge;
using CollectibleNode = Lisp::CollectibleNode;

CollectibleGraph::CollectibleGraph(const GarbageCollector & collector)
{
  std::unordered_set<std::shared_ptr<CollectibleNode>> todo;
  std::unordered_set<std::shared_ptr<CollectibleNode>> done;
  collector.forEachReachable([&todo, this](const Cell & cell) {
      auto node = std::make_shared<CollectibleNode>(cell);
      nodes[cell] = node;
      allNodes.push_back(node);
      todo.insert(node);
      if(cell.isRoot())
      {
        rootNodes.push_back(node);
      }
      else
      {
        bulkNodes.push_back(node);
      }
  });
  while(!todo.empty())
  {
    auto node = *todo.begin();
    todo.erase(node);
    done.insert(node);
    node->getCell().forEachChild([&done, &node, &todo, this](const Cell& child, std::size_t index) {
        if(child.isA<Collectible>())
        {
          auto itr = nodes.find(child);
          if(itr == nodes.end())
          {
            throw std::logic_error("node hasnt been added");
          }
          auto edge = std::make_shared<CollectibleEdge>(node.get(),
                                                        itr->second.get(),
                                                        index);
          node->children.push_back(edge.get());
          itr->second->parents.push_back(edge.get());
          edges.push_back(edge);
          if(todo.find(itr->second) == todo.end() &&
             done.find(itr->second) == done.end())
          {
            todo.insert(itr->second);
          }
        }
        else
        {
          leaves.push_back(std::make_pair(node, index));
        }
    });
  }
}

std::shared_ptr<CollectibleNode> CollectibleGraph::findNode(const Cell & cell) const
{
  auto itr = nodes.find(cell);
  if(itr != nodes.end())
  {
    return itr->second;
  }
  else
  {
    return nullptr;
  }
}

std::shared_ptr<CollectibleEdge> CollectibleGraph::getEdge(std::size_t index) const
{
  if(index < edges.size())
  {
    return edges[index];
  }
  else
  {
    return nullptr;
  }
}

void CollectibleGraph::forEachNode(std::function<void(const CollectibleNode & node)> func) const
{
  for(auto & p : nodes)
  {
    func(*p.second);
  }
}

void CollectibleGraph::forEachNode(std::function<void(const SharedNode & node)> func) const
{
  for(auto & p : nodes)
  {
    func(p.second);
  }
}
