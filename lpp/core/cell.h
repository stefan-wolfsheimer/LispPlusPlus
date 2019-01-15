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
#include <iostream>
#include <assert.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/managed_type.h>
#include <lpp/core/gc/color.h>

namespace Lisp
{
  class BasicCons;
  class Object;
  class Container;
  class Collectible;

  class Cell
  {
  public:
    friend class BasicCons;
    friend class Object;

    Cell();
    Cell(const Cell & rhs);
    Cell(IntegerType rhs);
    Cell(Collectible * rhs, TypeId typeId);
    
    template<typename T>
    Cell(T * obj);

    /**
     * deletes object with no reference, except for conses
     * conses are managed by the garbage collector
     */
    ~Cell();
    inline Cell& operator=(const Cell & rhs);

    inline TypeId getTypeId() const;
    std::string getTypeName() const;

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
    inline void forEachChild(std::function<void(const Cell&, std::size_t index)> func) const;
    void grey() const;

  protected:
    TypeId typeId;
    CellDataType data;
    inline void unset();
    inline void init(Collectible * cons, TypeId _typeId);
    inline void init(ManagedType * managedType, TypeId _typeId);
  };
}

std::ostream & operator<<(std::ostream & ost, const Lisp::Cell & cell);

//////////////////////////////////////////////////////////////////////
//
// hash function for Lisp::Cell
//
//////////////////////////////////////////////////////////////////////
namespace std
{
  template<>
  class hash<::Lisp::Cell>
  {
  public:
    size_t operator()(const ::Lisp::Cell & c) const
    {
      if(c.isA<const ::Lisp::Collectible>())
      {
        static hash<const ::Lisp::Collectible*> hasher;
        return hasher(c.as<const ::Lisp::Collectible>());
      }
      else if(c.isA<::Lisp::Nil>())
      {
        return 0u;
      }
    }
  };
}

//////////////////////////////////////////////////////////////////////
//
// equal_to for Lisp::Cell
//
//////////////////////////////////////////////////////////////////////
namespace std
{
  template<>
  class equal_to<::Lisp::Cell>
  {
  public:
    size_t operator()(const ::Lisp::Cell & a, const ::Lisp::Cell & b) const
    {
      if(a.isA<const ::Lisp::Collectible>() && b.isA<const ::Lisp::Collectible>())
      {
        static equal_to<const Lisp::Collectible*> eq;
        return eq(a.as<::Lisp::Collectible>(), b.as<::Lisp::Collectible>());
      }
      else if(a.isA<::Lisp::Nil>() && b.isA<::Lisp::Nil>())
      {
        return true;
      }
      return false;
    }
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

inline Lisp::Cell::Cell(const Cell & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<ManagedType>())
  {
    static_cast<ManagedType*>(data.ptr)->refCount++;
  }
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
  data.ptr = rhs;
}

inline Lisp::Cell& Lisp::Cell::operator=(const Cell & rhs)
{
  //assert(!rhs.isA<BasicCons>() || rhs.as<BasicCons>()->isRoot());
  unset();
  typeId = rhs.typeId;
  data = rhs.data;
  if(isA<ManagedType>())
  {
    static_cast<ManagedType*>(data.ptr)->refCount++;
  }
  return *this;
}

inline Lisp::Cell::~Cell()
{
  unset();
}

inline void Lisp::Cell::unset()
{
  if(TypeTraits<ManagedType>::isA(typeId))
  {
    ManagedType * obj = static_cast<ManagedType*>(data.ptr);
    assert(obj->refCount);
    if(! --obj->refCount)
    {
      delete obj;
      data.ptr = nullptr;
    }
  }
}


inline void Lisp::Cell::init(Lisp::ManagedType * obj,
                             Lisp::TypeId _typeId)
{
  typeId = _typeId;
  obj->refCount++;
  data.ptr = obj;
}

inline void Lisp::Cell::init(Lisp::Collectible * cons,
                             Lisp::TypeId _typeId)
{
  typeId = _typeId;
  data.ptr = cons;
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

inline void Lisp::Cell::forEachChild(std::function<void(const Cell&, std::size_t index)> func) const
{
  std::size_t index = 0;
  forEachChild([&index, &func](const Cell& cell) {
      func(cell, index);
      index++;
   });
}

