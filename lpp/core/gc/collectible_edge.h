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
#include <unordered_set>
#include <lpp/core/gc/collectible_node.h>

namespace Lisp
{
  class CollectibleNode;
  class Cell;

  class CollectibleEdge
  {
  public:
    CollectibleEdge(CollectibleNode * _parent,
                    CollectibleNode * _child,
                    std::size_t _index);
    inline Cell getParent() const;
    inline Cell getChild() const;
    inline std::size_t getIndex() const;
  private:
    const CollectibleNode * child;
    const CollectibleNode * parent;
    std::size_t index;
  };
}

inline Lisp::CollectibleEdge::CollectibleEdge(CollectibleNode * _parent,
                                              CollectibleNode * _child,
                                              std::size_t _index)
  : child(_child), parent(_parent), index(_index)
{
}

inline Lisp::Cell Lisp::CollectibleEdge::getParent() const
{
  return parent->getCell();
}

inline Lisp::Cell Lisp::CollectibleEdge::getChild() const
{
  return child->getCell();
}

inline std::size_t Lisp::CollectibleEdge::getIndex() const
{
  return index;
}
