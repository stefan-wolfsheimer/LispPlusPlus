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
#include <cstdint>
#include <lpp/core/cell.h>
#include "config.h"

namespace Lisp
{
  class Object : public Cell
  {
  public:
    Object();
    Object(const Object & rhs);
    Object(Object && rhs);
    explicit Object(const Cell & rhs);

    template<typename T>
    Object(T * obj);

    static Object nil();
    static Object undefined();

    Object & operator=(const Cell & rhs);
    Object & operator=(Cell && rhs);
    Object & operator=(const Object & rhs);
    Object & operator=(Object && rhs);
 
    ~Object();

    void swap(Object & rhs);
  protected:
    void init(BasicCons * cons, TypeId _typeId);
    void init(Container * cons, TypeId _typeId);
    inline void init(ManagedType * managedType, TypeId _typeId);
  private:
    inline void unsetCons();
  };

  extern Object nil;
  extern Object undefined;
}

inline Lisp::Object::Object() : Lisp::Cell()
{
}

inline Lisp::Object::Object(Object && rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  rhs.typeId = TypeTraits<Nil>::getTypeId();
}

template<typename T>
inline Lisp::Object::Object(T * obj)
{
  init(obj, TypeTraits<T>::getTypeId());
}

inline Lisp::Object Lisp::Object::nil()
{
  Object ret;
  ret.typeId = TypeTraits<Nil>::getTypeId();
  ret.data.pCons = nullptr;
  return ret;
}

inline Lisp::Object Lisp::Object::undefined()
{
  Object ret;
  ret.typeId = TypeTraits<Undefined>::getTypeId();
  ret.data.pCons = nullptr;
  return ret;
}

inline void Lisp::Object::init(ManagedType * managedType, TypeId _typeId)
{
  Cell::init(managedType, _typeId);
}
