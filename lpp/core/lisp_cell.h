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
#include <functional>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/managed_type.h>

namespace Lisp
{
  class Cons;
  class Object;

  class Cell
  {
  public:
    friend class Cons;
    Cell();
    Cell(const Cell & rhs);
    Cell(const Object & rhs);
    Cell(IntegerType rhs);

    template<typename T>
    Cell(T * obj);

    /**
     * deletes object with no reference, except for conses
     * conses are managed by the garbage collector
     */
    ~Cell();
    Cell& operator=(const Object & rhs);

    inline TypeId getTypeId() const;

    template<typename T>
    inline bool isA() const;

    template<typename T>
    inline typename Lisp::TypeTraits<T>::Type as() const;

    void forEachChild(std::function<void(const Cell&)> func) const;
  protected:
    TypeId typeId;
    CellDataType data;
    void unset();
    void init(Cons * cons, TypeId _typeId);
    inline void init(ManagedType * managedType, TypeId _typeId);
  };
}

//////////////////////////////////////////////////////////////////////
//
// Implementation
//
//////////////////////////////////////////////////////////////////////
inline Lisp::Cell::Cell()
{
  data.ptr = nullptr;
  typeId = Lisp::TypeTraits<Lisp::Nil>::typeId;
}

inline Lisp::Cell::Cell(Lisp::IntegerType value)
{
  typeId = Lisp::TypeTraits<Lisp::IntegerType>::typeId;
  data.intValue = value;
}

template<typename T>
inline Lisp::Cell::Cell(T * obj)
{
  init(obj, Lisp::TypeTraits<T>::typeId);
}

inline void Lisp::Cell::init(Lisp::ManagedType * obj,
                             Lisp::TypeId _typeId)
{
  typeId = _typeId;
  obj->refCount++;
  data.ptr = obj;
}

inline Lisp::TypeId Lisp::Cell::getTypeId() const
{
  return typeId;
}

template<typename T>
inline bool Lisp::Cell::isA() const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline typename Lisp::TypeTraits<T>::Type Lisp::Cell::as() const
{
  return TypeTraits<T>::as(data, typeId);
}

