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
#include <lpp/core/memory/color.h>
#include <lpp/core/config.h>

namespace Lisp
{
  template<typename T>
  class UnmanagedCollectibleContainer;

  template<typename T>
  class ColorMap;
  
  class Allocator;

  template<typename T>
  class CollectibleContainer
  {
  public:
    friend class ColorMap<T>;
    friend class UnmanagedCollectibleContainer<T>;

    CollectibleContainer(Color _color, bool _isRoot, Allocator * _gc);
    inline void remove(T * obj);
    inline void add(T * obj);
    inline void move(T * obj);

    inline T * popBack();
    inline T * back() const;
    inline bool empty() const;
    inline std::size_t size() const;
    inline bool isRoot() const;
    inline Color getColor() const;
    
    inline void root(T * obj);
    inline void unroot(T * obj);
    inline void collect();

    inline Allocator * getCollector() const;
    inline CollectibleContainer<T> * getOtherContainer() const;
    inline CollectibleContainer<T> * getGreyContainer() const;
    inline CollectibleContainer<T> * getToContainer() const;
    
    using const_iterator = typename std::vector<T*>::const_iterator;
    using iterator = typename std::vector<T*>::iterator;
    inline const_iterator cbegin() const noexcept;
    inline const_iterator cend() const noexcept;
    inline iterator begin() noexcept;
    inline iterator end() noexcept;
    inline T* operator[](std::size_t i) const;
  private:
    std::vector<T*> elements;
    CollectibleContainer<T> * otherElements;
    CollectibleContainer<T> * greyElements;
    CollectibleContainer<T> * toElements;
    Color color;
    bool _isRoot;
    Allocator * gc;
  };
}

////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline Lisp::CollectibleContainer<T>::CollectibleContainer(Color _color, bool __isRoot, Allocator * _gc)
  : color(_color), _isRoot(__isRoot), gc(_gc)
{
}

template<typename T>
inline void Lisp::CollectibleContainer<T>::remove(T * obj)
{
  assert(elements.size() > 0);
  assert(elements[obj->getIndex()] == obj);
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
inline void Lisp::CollectibleContainer<T>::move(T * obj)
{
  obj->container->remove(obj);
  add(obj);
}

template<typename T>
inline T * Lisp::CollectibleContainer<T>::popBack()
{
  T * ret = elements.back();
  elements.pop_back();
  return ret;
}

template<typename T>
inline T * Lisp::CollectibleContainer<T>::back() const
{
  return elements.back();
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
inline bool Lisp::CollectibleContainer<T>::isRoot() const
{
  return _isRoot;
}

template<typename T>
inline Lisp::Color Lisp::CollectibleContainer<T>::getColor() const
{
  return color;
}

#include <iostream>
template<typename T>
inline void Lisp::CollectibleContainer<T>::root(T * obj)
{
  assert(!obj->isRoot());
  assert(obj->index < elements.size());
  assert(obj == elements[obj->index]);
  remove(obj);
  obj->refCount = 1;
  otherElements->add(obj);
}

template<typename T>
inline void Lisp::CollectibleContainer<T>::unroot(T * obj)
{
  assert(obj->isRoot());
  assert(obj == elements[obj->index]);
  remove(obj);
  otherElements->add(obj);
}

template<typename T>
inline Lisp::Allocator * Lisp::CollectibleContainer<T>::getCollector() const
{
  return gc;
}

template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleContainer<T>::getOtherContainer() const
{
  return otherElements;
}

template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleContainer<T>::getGreyContainer() const
{
  return greyElements;
}


template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleContainer<T>::getToContainer() const
{
  return toElements;
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

template<typename T>
typename Lisp::CollectibleContainer<T>::iterator Lisp::CollectibleContainer<T>::begin() noexcept
{
  return elements.begin();
}

template<typename T>
typename Lisp::CollectibleContainer<T>::iterator Lisp::CollectibleContainer<T>::end() noexcept
{
  return elements.end();
}

template<typename T>
inline T* Lisp::CollectibleContainer<T>::operator[](std::size_t i) const
{
  return elements[i];
}
