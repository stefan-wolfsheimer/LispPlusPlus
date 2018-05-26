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
#pragma once
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <cstdint>

namespace Lisp
{
  class Cons;
  class ConsGraphEdge;
  class ConsGraphNode;

  //todo: remove this function and use garbagecollector method instead
  bool checkConsFactory(const ConsFactory & factory);
  //todo: remove this function and use garbagecollector method instead
  bool checkColorOfConses(const ConsFactory & factory,
                          Lisp::ConsFactory::Color color);

  class ConsGraph
  {
  public:
    ConsGraph(const ConsFactory & factory);
    std::shared_ptr<ConsGraphNode> getRootNode() const;
    std::shared_ptr<ConsGraphNode> findNode(const Cons *) const;
    std::shared_ptr<ConsGraphNode> getNode(std::size_t index) const;
    std::shared_ptr<ConsGraphEdge> findEdge(const Cons * parent,
                                            const Cons * child) const;
    std::shared_ptr<ConsGraphEdge> getEdge(std::size_t index) const;
    std::size_t numNodes() const;
    std::size_t numEdges() const;
  private:
    typedef std::shared_ptr<ConsGraphNode> SharedNode;
    typedef std::pair<const Cons*, const Cons *> ConsPair;
    void addEdge(ConsGraphNode * parent, ConsGraphNode * child);
    std::map<ConsPair, std::shared_ptr<ConsGraphEdge> > edges;
    std::unordered_map<const Cons*, SharedNode> nodes;
  };
}
