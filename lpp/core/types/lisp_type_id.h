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
#include <cstdint>
#include <memory>
#include <vector>

namespace Lisp
{
  /** typeId
   *  largest bit flags managed types
   *  00: cons, nil     (0x0000)
   *  01: value types   (0x4000)
   *  10: managed types (0x8000)
   *  11: container     (0xc000)
   */

  typedef ::std::uint_least16_t TypeId;
  typedef ::std::int_fast32_t IntegerType;
  class Symbol;
  class String;
  class Cons;
  class Cell;
  class ConsFactory;
  class BuiltinFunction;
  class Function;
  struct BasicType {};
  struct ValueType : BasicType {};
  struct Nil : BasicType {};
  struct Undefined : BasicType {};


  struct ContainerType : BasicType
  {
    //virtual ~ContainerType() {}
    //virtual bool stepGarbageCollector(ConsFactory * factory) = 0;
    //virtual std::vector<Cell> getChildren() const = 0;
  };
  
  class ManagedType : public BasicType
  {
  public:
    ManagedType();
    virtual ~ManagedType() {}
    inline std::size_t getRefCount() const;
  private:
    friend class Cell;
    std::size_t refCount;
  };
  struct AtomType : ValueType, public ManagedType {};

  union CellDataType
  {
    IntegerType intValue;
    BasicType * ptr;
  };

  template<typename T>
  struct TypeTraits
  {
  };

  template<TypeId TID>
  struct TypeIdTraits
  {
    static const TypeId typeId = TID;
    static inline bool isA(TypeId tid)
    {
      return tid == typeId;
    }
  };

  template<TypeId TID, typename T>
  struct PointerTypeTraits : public TypeIdTraits<TID>
  {
    typedef T * Type;
    static inline Type as(const CellDataType & data, TypeId tid)
    {
      if(TypeIdTraits<TID>::isA(tid))
      {
        return static_cast<T*>(data.ptr);
      }
      else
      {
        return nullptr;
      }
    }
  };

  template<TypeId TID, typename T>
  struct ValueTypeTraits : public TypeIdTraits<TID>
  {
  };

  template<>
  struct TypeTraits<Nil> : public TypeIdTraits<0x0000>
  {
  };

  template<>
  struct TypeTraits<Undefined> : public TypeIdTraits<0x0001>
  {
  };

  template<>
  struct TypeTraits<IntegerType> : TypeIdTraits<0x4001u>
  {
    typedef IntegerType Type;
    static inline Type as(const CellDataType & data, TypeId tid)
    {
      if(TypeTraits<IntegerType>::isA(tid))
      {
        return data.intValue;
      }
      else
      {
        return 0;
      }
    }
  };

  template<>
  struct TypeTraits<String> : PointerTypeTraits<0x8001u, String>
  {
  };

  template<>
  struct TypeTraits<Symbol> : PointerTypeTraits<0x8002u, Symbol>
  {
  };

  template<>
  struct TypeTraits<BuiltinFunction> : PointerTypeTraits<0x8003u,
                                                         BuiltinFunction>
  {
  };

  template<>
  struct TypeTraits<Function> : PointerTypeTraits<0x8004u, Function>
  {
  };

  template<>
  struct TypeTraits<Cons> : PointerTypeTraits<0x0002u, Cons>
  {
  };

  template<>
  struct TypeTraits<const Cons> : PointerTypeTraits<0x0002u, const Cons>
  {
  };


  template<>
  struct TypeTraits<ManagedType> : public TypeTraits<void>
  {
    typedef ManagedType * Type;
    static inline bool isA(TypeId tid)
    {
      return tid & 0x8000;
    }

    static inline Type as(const CellDataType & data, TypeId tid)
    {
      if(isA(tid))
      {
        return static_cast<ManagedType*>(data.ptr);
      }
      else
      {
        return nullptr;
      }
    }
  };

  template<>
  struct TypeTraits<ValueType>
  {
    static inline bool isA(TypeId tid)
    {
      return tid & 0x4000;
    }
  };

  template<>
  struct TypeTraits<AtomType>
  {
    static inline bool isA(TypeId tid)
    {
      // todo container type is not an atom
      // check differnt predicates in std scheme
      return tid & 0xc000;
    }
  };

  template<>
  struct TypeTraits<ContainerType>
  {
    static inline bool isA(TypeId tid)
    {
      // 00 ^ 11 = 11 -> false (cons, nil)
      // 01 ^ 11 = 10 -> false (value type)
      // 10 ^ 11 = 01 -> false (managed type)
      // 11 ^ 11 = 00 -> true (atom type)
      return !(tid ^ 0xc000);
    }
  };
}

///////////////////////////////////////////////////////////////////////
//
// ManagedType
//
///////////////////////////////////////////////////////////////////////
inline Lisp::ManagedType::ManagedType()
{
  refCount = 0;
}

inline std::size_t Lisp::ManagedType::getRefCount() const
{
  return refCount;
}



