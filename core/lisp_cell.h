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
#include "types/lisp_type_id.h"
#include "types/lisp_managed_type.h"
#include "types/lisp_nil.h"

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
    inline T * as() const;

  protected:
    TypeId typeId;
    union
    {
      int int_val;
      void * ptr;
    } data;
    void unset();
    inline void init(Cons * cons, TypeId _typeId);
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
  typeId = Lisp::Nil::typeId;
}

template<typename T>
inline Lisp::Cell::Cell(T * obj)
{
  init(obj, T::typeId);
}

inline void Lisp::Cell::init(Lisp::Cons * cons,
                             Lisp::TypeId _typeId)
{
  typeId = _typeId;
  data.ptr = cons;
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

namespace Lisp
{
  namespace Detail
  {
    template<typename T>
    struct TypeChecker
    {
      static inline bool isA(TypeId tid)
      {
        return T::typeId == tid;
      }
    };

    template<>
    struct TypeChecker<ManagedType>
    {
      static inline bool isA(TypeId tid)
      {
        return isManagedTypeId(tid);
      }
    };
  }
}

template<typename T>
inline bool Lisp::Cell::isA() const
{
  return Detail::TypeChecker<T>::isA(typeId);
}

template<typename T>
inline T * Lisp::Cell::as() const
{
  if(isA<T>())
  {
    return (T*)data.ptr;
  }
  else
  {
    return nullptr;
  }
}
