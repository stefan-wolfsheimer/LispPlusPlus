/******************************************************************************
Copyright (c) 2019, Stefan Wolfsheimer

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
#include <lpp/core/cell_data_type.h>

namespace Lisp
{
  struct AtomStorageTrait {};
  struct ManagedStorageTrait {};
  struct ConsStorageTrait {};
  struct ContainerStorageTrait {};
  struct SymbolStorageTrait {};

  /* ConsStorageTrait or ContainerStorageTrait
   */
  struct CollectibleStorageTrait {};

  namespace Traits
  {
    ///////////////////////////////////////////////////////////////////////////
    //
    // type matcher
    //
    ///////////////////////////////////////////////////////////////////////////

    /**
     * Matches exact type id
     */
    template<TypeId TID>
    struct Id
    {
      static constexpr TypeId getTypeId()
      {
        return TID;
      }

      static inline bool isA(TypeId tid)
      {
        return tid == TID;
      }
    };

    template<typename IS_MANAGED, typename IS_CONTAINER>
    struct PolymorphicId
    {
      using IsPolymorphic = std::true_type;
    };


    /**
     * Matches type by mask
     */
    template<TypeId TID>
    struct IdMask
    {
      static const TypeId typeId = TID;

      static inline bool isA(TypeId tid)
      {
        return (tid & 0xc000) == TID;
      }
    };

    /**
     * Matches type id less than value
     */
    template<TypeId TID>
    struct IdLt
    {
      static const TypeId typeId = TID;

      static inline bool isA(TypeId tid)
      {
        return (tid < TID);
      }
    };

    /**
     * Matches type id greater than value
     */
    template<TypeId TID>
    struct IdGt
    {
      static const TypeId typeId = TID;

      static inline bool isA(TypeId tid)
      {
        return (tid > TID);
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // Basic type traits
    //
    ///////////////////////////////////////////////////////////////////////////
    template<typename CLS, typename TypeMatcher>
    struct Integer : public TypeMatcher
    {
      using Type = CLS;
      using StorageTrait = AtomStorageTrait;
      using IsPolymorphic = std::false_type;

      static inline IntegerType as(const CellDataType & data, TypeId tid)
      {
        if(TypeMatcher::isA(tid))
        {
          return data.intValue;
        }
        else
        {
          return 0;
        }
      }
    };

    template<typename T, typename TypeMatcher>
    struct Null : public TypeMatcher
    {
      using Type = T*;
      using StorageTrait = AtomStorageTrait;
      using IsPolymorphic = std::false_type;

      static inline void * as(const CellDataType & data, TypeId tid)
      {
        return nullptr;
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Managed Type
    ///////////////////////////////////////////////////////////////////////////
    template<typename T, typename TypeMatcher, typename POLY=std::false_type>
    struct ManagedType : public TypeMatcher
    {
      using Type = T*;
      using StorageTrait = ManagedStorageTrait;
      using IsPolymorphic = POLY;

      static inline Type as(const CellDataType & data, TypeId tid)
      {
        if(TypeMatcher::isA(tid))
        {
          return dynamic_cast<T*>(data.pManaged);
        }
        else
        {
          return nullptr;
        }
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Cons Type
    ///////////////////////////////////////////////////////////////////////////
    template<typename T, typename TypeMatcher>
    struct BasicCons : public TypeMatcher
    {
      using Type = T*;
      using StorageTrait = ConsStorageTrait;
      using IsPolymorphic = std::false_type;

      static inline Type as(const CellDataType & data, TypeId tid)
      {
        if(TypeMatcher::isA(tid))
        {
          return static_cast<T*>(data.pCons);
        }
        else
        {
          return nullptr;
        }
      }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Container Type
    ///////////////////////////////////////////////////////////////////////////
    template<typename T, typename TypeMatcher, typename POLY=std::false_type>
    struct Container : public TypeMatcher
    {
      using Type = T*;
      using StorageTrait = ContainerStorageTrait;
      using IsPolymorphic = POLY;

      static inline Type as(const CellDataType & data, TypeId tid)
      {
        if(TypeMatcher::isA(tid))
        {
          return dynamic_cast<T*>(data.pContainer);
        }
        else
        {
          return nullptr;
        }
      }
    };


    template<typename T, typename IS_MANAGED, typename IS_CONTAINER>
    struct Polymorphic
    {
    };

  }
}

