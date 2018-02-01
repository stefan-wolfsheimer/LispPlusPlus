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
#include "lisp_cons_graph_edge.h"
#include "lisp_cons_graph_node.h"
#include "core/lisp_cons.h"
#include <unordered_set>

using ConsGraphNode = Lisp::ConsGraphNode;
using ConsGraphEdge = Lisp::ConsGraphEdge;
using ConsGraphNodeSet = std::unordered_set<const ConsGraphNode*>;

ConsGraphNodeSet ConsGraphEdge::computeChildSet(const ConsGraphNodeSet & input)
{
  ConsGraphNodeSet childSet;
  ConsGraphNodeSet todo(input.begin(), input.end());
  while(!todo.empty())
  {
    auto node = *todo.begin();
    todo.erase(node);
    childSet.insert(node);
    for(auto child : node->children)
    {
      if(childSet.find(child) == childSet.end())
      {
        todo.insert(child);
      }
    }
  }
  return childSet;
}

Lisp::ConsGraphEdge::ConsGraphEdge(const Lisp::ConsGraphNode * _parent,
                                   const Lisp::ConsGraphNode * _child)
  : weight(0), weightReady(false), child(_child), parent(_parent)
{
}

std::size_t Lisp::ConsGraphEdge::getWeight() const
{
  if(!weightReady)
  {
    // compute all children reachable from this
    auto childSet = computeChildSet({child});
    // compute all parents of childSet
    ConsGraphNodeSet parentSet;
    for(const ConsGraphNode* child : childSet)
    {
      for(const ConsGraphNode* parent : child->parents)
      {
        if(childSet.find(parent) == childSet.end() && (child != this->child || parent != this->parent))
        {
          parentSet.insert(parent);
        }
      }
    }
    // all children of parent set
    parentSet = computeChildSet(parentSet);

    // compute diff: parentSet -> parentSet - childSet
    // childSet -> childSet - all children of parentSet
    for(const ConsGraphNode* child : parentSet)
    {
      childSet.erase(child);
    }
    weight = childSet.size();
    weightReady = true;
  }
  return weight;
}

Lisp::Cons * Lisp::ConsGraphEdge::getParent() const
{
  return parent->getCons();
}

Lisp::Cons * Lisp::ConsGraphEdge::getChild() const
{
  return child->getCons();
}

