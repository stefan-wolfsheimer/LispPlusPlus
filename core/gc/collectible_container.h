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
#include "core/gc/color.h"

namespace Lisp
{
  class ConsFactory; //todo rename to GarbageCollector

  template<typename T>
  class CollectibleContainer
  {
  public:
    friend class ConsFactory; // todo remove friendship
    CollectibleContainer(Color _color, ConsFactory * _collector);
    inline Color getColor() const;
    inline ConsFactory * getCollector() const;
    inline void remove(T * obj);
    inline void add(T * obj);
    inline T * popBack();
    inline bool empty() const;
    inline std::size_t size() const;
  private:
    std::vector<T*> elements;
    ConsFactory * collector;
    Color color;
  };
}

template<typename T>
inline Lisp::CollectibleContainer<T>::CollectibleContainer(Color _color,
                                                           ConsFactory * coll) :
  color(_color), collector(coll)
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
  obj->color = color;
  obj->index = elements.size();
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
