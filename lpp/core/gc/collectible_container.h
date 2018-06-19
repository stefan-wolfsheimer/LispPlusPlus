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
#include <lpp/core/lisp_cell.h>
#include <lpp/core/gc/basic_collectible_container.h>

namespace Lisp
{
  template<typename T>
  class UnmanagedCollectibleContainer;

  class GarbageCollector;

  template<typename T>
  class CollectibleContainer : public CollectibleContainer<void>
  {
  public:
    friend class GarbageCollector;
    friend class UnmanagedCollectibleContainer<T>;
    CollectibleContainer(Color _color, GarbageCollector * _gc);
    inline void remove(T * obj);
    inline void add(T * obj);
    inline T * popBack();
    inline bool empty() const;
    inline std::size_t size() const;
    inline void addTo(std::vector<Cell> & cells) const;

    inline void root(T * obj);
    inline void unroot(T * obj);
    
    using const_iterator = typename std::vector<T*>::const_iterator;
    inline const_iterator cbegin() const noexcept;
    inline const_iterator cend() const noexcept;

  private:
    std::vector<T*> elements;
  };
}

////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline Lisp::CollectibleContainer<T>::CollectibleContainer(Color _color, GarbageCollector * _gc)
  : CollectibleContainer<void>(_color, _gc)
{
}

template<typename T>
inline void Lisp::CollectibleContainer<T>::remove(T * obj)
{
  assert(elements.size() > 0);
  assert(elements[obj->index] == obj);
  std::size_t index = obj->index;
  elements[obj->index] = elements.back();
  elements[obj->index]->index = index;
  elements.pop_back();
}

template<typename T>
inline void Lisp::CollectibleContainer<T>::add(T * obj)
{
  obj->index = elements.size();
  obj->container = this;
  elements.push_back(obj);
}

template<typename T>
inline T * Lisp::CollectibleContainer<T>::popBack()
{
  T * ret = elements.back();
  elements.pop_back();
  return ret;
}

template<typename T>
inline bool Lisp::CollectibleContainer<T>::empty() const
{
  return elements.empty();
}

template<typename T>
inline std::size_t Lisp::CollectibleContainer<T>::size() const
{
  return elements.size();
}

template<typename T>
void Lisp::CollectibleContainer<T>::addTo(std::vector<Cell> & cells) const
{
  for(auto p : elements)
  {
    cells.push_back(p);
  }
}

template<typename T>
inline void Lisp::CollectibleContainer<T>::root(T * obj)
{
  assert(!obj->isRoot());
  assert(obj->index < elements.size());
  assert(obj == elements[obj->index]);
  remove(obj);
  obj->refCount = 1;
  getOtherContainer<T>()->add(obj);
  collect();
}

template<typename T>
inline void Lisp::CollectibleContainer<T>::unroot(T * obj)
{
  assert(obj->isRoot());
  assert(obj == elements[obj->index]);
  remove(obj);
  getOtherContainer<T>()->add(obj);
  collect();
}

template<typename T>
typename Lisp::CollectibleContainer<T>::const_iterator Lisp::CollectibleContainer<T>::cbegin() const noexcept
{
  return elements.cbegin();
}

template<typename T>
typename Lisp::CollectibleContainer<T>::const_iterator Lisp::CollectibleContainer<T>::cend() const noexcept
{
  return elements.cend();
}
