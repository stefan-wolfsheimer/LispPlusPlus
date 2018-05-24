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
#include <vector>
#include <assert.h>
#include <lpp/core/gc/color.h>
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/lisp_cell.h>

namespace Lisp
{
  template<typename T>
  class UnmanagedCollectibleContainer
  {
  public:
    inline void move(Lisp::CollectibleContainer<T> & rhs);
    inline bool empty() const;
    inline std::size_t size() const;
    inline void addTo(std::vector<Cell> & cells) const;
    T * popBack();
  private:
    std::vector<std::vector<T*> > elements;
  };
}

template<typename T>
inline void Lisp::UnmanagedCollectibleContainer<T>
::move(Lisp::CollectibleContainer<T> & rhs)
{
  if(!rhs.elements.empty())
  {
    elements.push_back(std::vector<T*>());
    rhs.elements.swap(elements.back());
  }
}

template<typename T>
inline bool Lisp::UnmanagedCollectibleContainer<T>::empty() const
{
  return elements.empty();
}

template<typename T>
inline std::size_t Lisp::UnmanagedCollectibleContainer<T>::size() const
{
  std::size_t n = 0;
  for(auto & v : elements)
  {
    n+= v.size();
  }
  return n;
}

template<typename T>
inline void
Lisp::UnmanagedCollectibleContainer<T>::addTo(std::vector<Cell> & cells) const
{
  for(auto & v : elements)
  {
    for(auto & c : v)
    {
      cells.push_back(Cell(c));
    }
  }
}

template<typename T>
T * Lisp::UnmanagedCollectibleContainer<T>::popBack()
{
  if(elements.empty())
  {
    return nullptr;
  }
  else
  {
    T * ret = elements.back().back();
    elements.back().pop_back();
    if(elements.back().empty())
    {
      elements.pop_back();
    }
    return ret;
  }
}
