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
#include "core/lisp_cons_factory.h"
#include "core/lisp_cons.h"
#include "lisp_cons_graph.h"
#include "lisp_cons_graph_edge.h"
#include "lisp_cons_graph_node.h"


using Cons = Lisp::Cons;
using ConsGraphNode = Lisp::ConsGraphNode;
using ConsGraphEdge = Lisp::ConsGraphEdge;
using ConsFactory = Lisp::ConsFactory;
using Color = ConsFactory::Color;
using ConsGraph = Lisp::ConsGraph;
using SharedNode = std::shared_ptr<ConsGraphNode>;
using SharedEdge = std::shared_ptr<ConsGraphEdge>;

bool Lisp::checkColorOfConses(const ConsFactory & factory, Color color)
{
  auto conses = factory.getConses(color);
  std::size_t i = 0;
  for(const Cons * cons : conses)
  {
    if(cons->getColor() != color)
    {
      return false;
    }
    if(color != Color::Void && color != Color::Free)
    {
      if(cons->getIndex() != i)
      {
        return false;
      }
    }
    i++;
  }
  if(color == factory.getToColor())
  {
    for(auto cons : conses)
    {
      if(cons->getCarCell().isA<const Cons>())
      {
        if(cons->getCarCell().as<const Cons>()->getColor() == factory.getFromColor())
        {
          return false;
        }
      }
      if(cons->getCdrCell().isA<const Cons>())
      {
        if(cons->getCdrCell().as<const Cons>()->getColor() == factory.getFromColor())
        {
          return false;
        }
      }
    }
  }
  return true;
}

bool Lisp::checkConsFactory(const ConsFactory & factory)
{
  bool ret = true;
  std::vector<Color> colors({Color::Void,
                             Color::White,
                             Color::Grey,
                             Color::Black,
                             Color::WhiteRoot,
                             Color::GreyRoot,
                             Color::BlackRoot,
                             Color::Free});

  for(auto color : colors)
  {
    ret&= checkColorOfConses(factory, color);
  }
  return true;
}

ConsGraph::ConsGraph(const Lisp::ConsFactory & factory)
{
  std::unordered_set<const Cons*> reachable = factory.getReachableConsesAsConstSet();
  nodes[nullptr] = std::make_shared<ConsGraphNode>(nullptr);
  for(auto cons : reachable)
  {
    nodes[cons] = std::make_shared<ConsGraphNode>(cons);
  }
  for(auto cons : reachable)
  {
    if(cons->getCarCell().isA<Cons>())
    {
      addEdge(nodes[cons].get(), nodes[cons->getCarCell().as<Cons>()].get());
    }
    if(cons->getCdrCell().isA<Cons>())
    {
      addEdge(nodes[cons].get(), nodes[cons->getCdrCell().as<Cons>()].get());
    }
  }
  for(auto cons : factory.getRootConses())
  {
    addEdge(nodes[nullptr].get(), nodes[cons].get());
  }
}

void ConsGraph::addEdge(ConsGraphNode * parent, ConsGraphNode * child)
{
  const Cons * p = parent ? parent->getCons() : nullptr;
  const Cons * c = child ? child->getCons() : nullptr;
  if(parent && child)
  {
    parent->children.insert(child);
    child->parents.insert(parent);
  }
  edges.insert(std::make_pair(ConsPair(p, c),
                              std::make_shared<ConsGraphEdge>(parent, child)));
}

SharedNode ConsGraph::getRootNode() const
{
  auto itr = nodes.find(nullptr);
  return itr->second;
}

SharedNode ConsGraph::findNode(const Cons * cons) const
{
  auto itr = nodes.find(cons);
  if(itr != nodes.end())
  {
    return itr->second;
  }
  else
  {
    return SharedNode();
  }
}

SharedEdge ConsGraph::findEdge(const Cons * parent,
                               const Cons * child) const
{
  auto itr = edges.find(ConsPair(parent, child));
  if(itr != edges.end())
  {
    return itr->second;
  }
  else
  {
    return SharedEdge();
  }
}

