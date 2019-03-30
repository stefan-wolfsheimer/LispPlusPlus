/******************************************************************************
Copyright (c) 2017-2019, Stefan Wolfsheimer

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
#include <lpp/core/memory/color.h>

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
    Cell(BasicCons * rhs, TypeId typeId);
    Cell(Container * rhs, TypeId typeId);

    //Cell(Collectible * rhs, TypeId typeId);
    
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

    bool operator==(const Lisp::Cell & b) const;

    /**
     * Hash value of the cell content.
     * @todo complete for more types
     */
    size_t hash() const;

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
    inline void init(BasicCons * cons, TypeId _typeId);
    inline void init(Container * container, TypeId _typeId);
    inline void init(ManagedType * managedType, TypeId _typeId);
  private:
    template<typename T>
    inline bool _isA(AtomStorageTrait, std::false_type) const;

    template<typename T>
    inline bool _isA(ManagedStorageTrait, std::false_type) const;

    template<typename T>
    inline bool _isA(ConsStorageTrait, std::false_type) const;

    template<typename T>
    inline bool _isA(CollectibleStorageTrait, std::false_type) const;

    template<typename T>
    inline bool _isA(ContainerStorageTrait, std::false_type) const;

    template<typename T>
    inline bool _isA(SymbolStorageTrait, std::false_type) const;

    template<typename T>
    inline bool _isA(ManagedStorageTrait, std::true_type) const;

    template<typename T>
    inline bool _isA(ContainerStorageTrait, std::true_type) const;
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
    inline size_t operator()(const ::Lisp::Cell & c) const
    {
      return c.hash();
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
    inline size_t operator()(const ::Lisp::Cell & a, const ::Lisp::Cell & b) const
    {
      return a == b;
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
  data.pCons = nullptr;
  typeId = Lisp::TypeTraits<Lisp::Nil>::getTypeId();
}

inline Lisp::Cell::Cell(Lisp::IntegerType value)
{
  typeId = Lisp::TypeTraits<Lisp::IntegerType>::getTypeId();
  data.intValue = value;
}

inline Lisp::Cell::Cell(const Cell & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<ManagedType>())
  {
    static_cast<ManagedType*>(data.pManaged)->refCount++;
  }
}

template<typename T>
inline Lisp::Cell::Cell(T * obj)
{
  init(obj, Lisp::TypeTraits<T>::getTypeId());
}

inline Lisp::Cell::Cell(BasicCons * rhs, TypeId _typeId) : typeId(_typeId)
{
  assert(Lisp::TypeTraits<BasicCons>::isA(_typeId));
  data.pCons = rhs;
}

inline Lisp::Cell::Cell(Container * rhs, TypeId _typeId) : typeId(_typeId)
{
  assert(Lisp::TypeTraits<Container>::isA(_typeId));
  data.pContainer = rhs;
}

inline Lisp::Cell& Lisp::Cell::operator=(const Cell & rhs)
{
  //assert(!rhs.isA<BasicCons>() || rhs.as<BasicCons>()->isRoot());
  unset();
  typeId = rhs.typeId;
  data = rhs.data;
  if(isA<ManagedType>())
  {
    data.pManaged->refCount++;
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
    assert(data.pManaged->refCount);
    if(! --data.pManaged->refCount)
    {
      delete data.pManaged;
      data.pManaged = nullptr;
    }
  }
}

inline void Lisp::Cell::init(Lisp::ManagedType * obj, Lisp::TypeId _typeId)
{
  assert(Lisp::TypeTraits<ManagedType>::isA(_typeId));
  typeId = _typeId;
  obj->refCount++;
  data.pManaged = obj;
}

inline void Lisp::Cell::init(Lisp::Container * obj, Lisp::TypeId _typeId)
{
  assert(Lisp::TypeTraits<Container>::isA(_typeId));
  typeId = _typeId;
  data.pContainer = obj;
}

inline void Lisp::Cell::init(Lisp::BasicCons * obj, Lisp::TypeId _typeId)
{
  assert(Lisp::TypeTraits<BasicCons>::isA(_typeId));
  typeId = _typeId;
  data.pCons = obj;
}

inline Lisp::TypeId Lisp::Cell::getTypeId() const
{
  return typeId;
}

template<typename T>
inline bool Lisp::Cell::isA() const
{
  return _isA<T>(typename TypeTraits<T>::StorageTrait(),
                 typename TypeTraits<T>::IsPolymorphic());
}

template<typename T>
inline bool Lisp::Cell::_isA(AtomStorageTrait, std::false_type) const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline bool Lisp::Cell::_isA(ManagedStorageTrait, std::false_type) const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline bool Lisp::Cell::_isA(ConsStorageTrait, std::false_type) const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline bool Lisp::Cell::_isA(ContainerStorageTrait, std::false_type) const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline bool Lisp::Cell::_isA(SymbolStorageTrait, std::false_type) const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline bool Lisp::Cell::_isA(CollectibleStorageTrait, std::false_type) const
{
  return TypeTraits<T>::isA(typeId);
}

template<typename T>
inline bool Lisp::Cell::_isA(ManagedStorageTrait, std::true_type) const
{
  return TypeTraits<T>::isA(typeId) && (dynamic_cast<const T*>(data.pManaged) != nullptr);
}

template<typename T>
inline bool Lisp::Cell::_isA(ContainerStorageTrait, std::true_type) const
{
  return TypeTraits<T>::isA(typeId) && (dynamic_cast<const T*>(data.pContainer) != nullptr);
}



template<typename T>
inline typename Lisp::TypeTraits<T>::Type Lisp::Cell::as() const
{
  return TypeTraits<T>::as(data, typeId);
}

inline void Lisp::Cell::forEachChild(std::function<void(const Cell&, std::size_t index)> func) const
{
  std::size_t index = 0;
  forEachChild([&index, &func](const Cell& cell) {
      func(cell, index);
      index++;
   });
}
