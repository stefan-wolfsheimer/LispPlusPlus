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
#include <lpp/core/gc/color.h>

namespace Lisp
{
  class Cons;
  class Object;
  class Container;
  class Collectible;

  class Cell
  {
  public:
    friend class Cons;
    Cell();
    Cell(const Cell & rhs);
    Cell(const Object & rhs);
    Cell(IntegerType rhs);
    Cell(Collectible * rhs, TypeId typeId);
    
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

    inline bool operator==(const Lisp::Cell & b) const;

    /**
     * Operations for Collectible */
    bool isRoot() const;
    Color getColor() const;
    std::size_t getRefCount() const;
    bool checkIndex() const;
    void forEachChild(std::function<void(const Cell&)> func) const;
    void grey() const;

  protected:
    TypeId typeId;
    CellDataType data;
    void unset();
    void init(Collectible * cons, TypeId _typeId);
    inline void init(ManagedType * managedType, TypeId _typeId);
  };
}

//////////////////////////////////////////////////////////////////////
//
// hash function for Lisp::Cell
//
//////////////////////////////////////////////////////////////////////
template<>
class std::hash<Lisp::Cell>
{
public:
  std::size_t operator()(const Lisp::Cell & c) const
  {
    if(c.isA<const Lisp::Collectible>())
    {
      static std::hash<const Lisp::Collectible*> hasher;
      return hasher(c.as<const Lisp::Collectible>());
    }
    else if(c.isA<Lisp::Nil>())
    {
      return 0u;
    }
  }
};

//////////////////////////////////////////////////////////////////////
//
// equal_to for Lisp::Cell
//
//////////////////////////////////////////////////////////////////////
template<>
class std::equal_to<Lisp::Cell>
{
public:
  std::size_t operator()(const Lisp::Cell & a, const Lisp::Cell & b) const
  {
    if(a.isA<const Lisp::Collectible>() && b.isA<const Lisp::Collectible>())
    {
      static std::equal_to<const Lisp::Collectible*> eq;
      return eq(a.as<Lisp::Collectible>(), b.as<Lisp::Collectible>());
    }
    else if(a.isA<Lisp::Nil>() && b.isA<Lisp::Nil>())
    {
      return true;
    }
    return false;
  }
};

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

inline Lisp::Cell::Cell(Collectible * rhs, TypeId _typeId)
  : typeId(_typeId)
{
  typeId = _typeId;
  data.ptr = reinterpret_cast<BasicType*>(rhs);
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

inline bool Lisp::Cell::operator==(const Lisp::Cell & b) const
{
  static std::equal_to<Lisp::Cell> eq;
  return eq(*this, b);
}


