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
#include <cstdint>
#include <memory>
#include <vector>
#include <lpp/core/cell_data_type.h>
#include <lpp/core/types/type_traits.h>

#define POLYMORPHIC_OBJECT_TYPE_ID 0xbfffu
#define POLYMORPHIC_CONTAINER_TYPE_ID 0xcfffu

#define DEF_TRAITS(CLS, ID, BASE)                                       \
 template<>                                                             \
 struct TypeTraits<CLS> : BASE<CLS, Traits::Id<ID>> {};                 \
 template<>                                                             \
 struct TypeTraits<const CLS> : BASE<const CLS, Traits::Id<ID>> {}

#define DEF_TRAITS_MATCH(CLS, ID, BASE)                                 \
   template<>                                                           \
 struct TypeTraits<CLS> : BASE<CLS,                                     \
                               Traits::IdMask<ID>> {};                  \
   template<>                                                           \
   struct TypeTraits<const CLS> : BASE<const CLS,                       \
                                       Traits::IdMask<ID>> {}

#define DEF_TRAITS_LT(CLS, ID, BASE)                                    \
 template<>                                                             \
 struct TypeTraits<CLS> : BASE<CLS,                                     \
                               Traits::IdLt<ID>> {};                    \
   template<>                                                           \
   struct TypeTraits<const CLS> : BASE<const CLS,                       \
                                       Traits::IdLt<ID>> {}

#define DEF_TRAITS_GT(CLS, ID, BASE)                                    \
 template<>                                                             \
 struct TypeTraits<CLS> : BASE<CLS,                                     \
                               Traits::IdLt<ID>> {};                    \
   template<>                                                           \
   struct TypeTraits<const CLS> : BASE<const CLS,                       \
                                       Traits::IdGt<ID>> {}

namespace Lisp
{

  /** typeId
   *  largest bit flags managed types
   *  00: value types             (0x0000)
   *  01: cons types              (0x4000)
   *      011 reference           (0x6000)
   *  10: managed types           (0x8000)
   *  11: collectibleContainer    (0xc000)
   * 
   *
   *
   *          Collectible
   *           /        \
   *     BasicCons    Container
   *                     \
   *                    Array
   */

  struct Any {};

  /* collectible types (container or BasicCons or Container) */
  class Collectible;

  /* value types */
  struct ValueType;
  struct Nil;
  class BasicType;
  struct Undefined;

  /* managed types */
  class ManagedType;
  class String;
  class Symbol;
  class PolymorphicObject;

  /* conses types */
  class BasicCons;
  class Cons;
  class Reference;

  /* container */
  class Container;
  class Array;
  class Function;
  class PolymorphicContainer;

  template<typename CLS>
  struct Traits::Polymorphic<CLS,
                             std::true_type, std::false_type> : Traits::ManagedType<CLS,
                                                                                    Id<POLYMORPHIC_OBJECT_TYPE_ID>,
                                                                                    std::true_type>
  {
  };

  template<typename CLS>
  struct Traits::Polymorphic<CLS,
                             std::false_type, std::true_type> : Traits::Container<CLS,
                                                                                  Id<POLYMORPHIC_CONTAINER_TYPE_ID>,
                                                                                  std::true_type>
  {
  };

  template<typename CLS>
  struct TypeTraits : Traits::Polymorphic<CLS,
                                          typename std::is_base_of<ManagedType, CLS>::type,
                                          typename std::is_base_of<Container, CLS>::type>
  {
  };

  DEF_TRAITS_LT(ValueType,      0x4000u, Traits::Null);
  DEF_TRAITS(Nil,               0x0000u, Traits::Null);
  DEF_TRAITS(Undefined,         0x0001u, Traits::Null);
  DEF_TRAITS(BasicType,         0x0002u, Traits::Integer);
  DEF_TRAITS(IntegerType,       0x0003u, Traits::Integer);

  // conses
  DEF_TRAITS_MATCH(BasicCons,   0x4000u,                          Traits::BasicCons);
  DEF_TRAITS(Cons,              0x4001u,                          Traits::BasicCons);
  DEF_TRAITS(Reference,         0x4002u,                          Traits::BasicCons);

  // objects
  DEF_TRAITS_MATCH(ManagedType, 0x8000u,                          Traits::ManagedType);
  DEF_TRAITS(String,            0x8001u,                          Traits::ManagedType);
  DEF_TRAITS(Symbol,            0x8002u,                          Traits::ManagedType);
  DEF_TRAITS(PolymorphicObject, POLYMORPHIC_OBJECT_TYPE_ID,       Traits::ManagedType);

  // containers
  DEF_TRAITS_MATCH(Container,      0xc000u,                       Traits::Container);
  DEF_TRAITS(Array,                0xc001u,                       Traits::Container);
  DEF_TRAITS(Function,             0xc002u,                       Traits::Container);
  DEF_TRAITS(PolymorphicContainer, POLYMORPHIC_CONTAINER_TYPE_ID, Traits::Container);

  /* Collectible TypeTraits
   */
  template<>
  struct TypeTraits<Collectible> : Traits::IdGt<0x0fffu>
  {
    using StorageTrait = CollectibleStorageTrait;
    using IsPolymorphic = std::false_type;
  };

  template<>
  struct TypeTraits<const Collectible> : Traits::IdGt<0x0fffu>
  {
    using StorageTrait = CollectibleStorageTrait;
    using IsPolymorphic = std::false_type;
  };

  template<>
  struct TypeTraits<Any>
  {
    using StorageTrait = AtomStorageTrait;
    using IsPolymorphic = std::false_type;

    static inline bool isA(TypeId tid)
    {
      return true;
    }

  };

  template<>
  struct TypeTraits<const Any>
  {
    using StorageTrait = AtomStorageTrait;
    using IsPolymorphic = std::false_type;

    static inline bool isA(TypeId tid)
    {
      return true;
    }
  };
}

#undef DEF_TRAITS
#undef DEF_TRAITS_MATCH
#undef DEF_TRAITS_LT
#undef DEF_TRAITS_GT

#undef POLYMORPHIC_OBJECT_TYPE_ID
#undef POLYMORPHIC_CONTAINER_TYPE_ID
