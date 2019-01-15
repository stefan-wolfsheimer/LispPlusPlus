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

#define DEF_TRAITS_NUL(CLS, ID)                                       \
  template<>                                                          \
  struct TypeTraits<CLS> : Traits::Null<Traits::Id<ID>> {};           \
  template<>                                                          \
  struct TypeTraits<const CLS> : Traits::Null<Traits::Id<ID>> {}

#define DEF_TRAITS_NUL_MATCH(CLS, ID)                                   \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Null<Traits::IdMask<ID>> {};          \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Null<Traits::IdMask<ID>> {}

#define DEF_TRAITS_INT(CLS, ID)                                         \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Integer<Traits::Id<ID>> {};           \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Integer<Traits::Id<ID>> {}      \

#define DEF_TRAITS_PTR(CLS, ID)                                         \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Pointer<CLS, Traits::Id<ID>> {};      \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Pointer<const CLS, Traits::Id<ID>> {}

#define DEF_TRAITS_PTR_MATCH(CLS, ID)                                   \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Pointer<CLS, Traits::IdMask<ID>> {};  \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Pointer<const CLS, Traits::IdMask<ID>> {}

#define DEF_TRAITS_PTR_LT(CLS, ID)                                      \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Pointer<CLS, Traits::IdLt<ID>> {};    \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Pointer<const CLS, Traits::IdLt<ID>> {}

#define DEF_TRAITS_PTR_GT(CLS, ID)                                      \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Pointer<CLS, Traits::IdGt<ID>> {};    \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Pointer<const CLS, Traits::IdGt<ID>> {}

#define DEF_TRAITS_NUL_LT(CLS, ID)                                      \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Null<Traits::IdLt<ID>> {};            \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Null<Traits::IdLt<ID>> {}

#define DEF_TRAITS_NUL_GT(CLS, ID)                                      \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::Null<Traits::IdGt<ID>> {};            \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::Null<Traits::IdGt<ID>> {}

#define DEF_TRAITS_PTR_CHOIC(CLS, C1, C2)                               \
 template<>                                                             \
 struct TypeTraits<CLS> : Traits::PointerChoice<CLS, C1, C2> {};        \
 template<>                                                             \
 struct TypeTraits<const CLS> : Traits::PointerChoice<const CLS, const C1, const C2> {}



namespace Lisp
{
  /** typeId
   *  largest bit flags managed types
   *  00: value types             (0x0000)
   *  01: cons types              (0x4000)
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

  /* collectible types (container or BasicCons or Container) */
  class Collectible;

  /* value types */
  struct ValueType;
  struct Nil;
  struct Undefined;

  /* conses types */
  class BasicCons;
  class Cons;
  class Reference;

  /* managed types */
  class ManagedType;
  class String;
  class Symbol;
  class Form;
  class BuiltinFunction;
  class Function;

  /* collectible */
  class Container;
  class Array;

  DEF_TRAITS_NUL_LT(ValueType,      0x4000u);
  DEF_TRAITS_NUL(Nil,               0x0000u);
  DEF_TRAITS_NUL(Undefined,         0x0001u);
  DEF_TRAITS_INT(IntegerType,       0x0002u);

  // conses
  DEF_TRAITS_PTR_MATCH(BasicCons,   0x4000u);
  DEF_TRAITS_PTR(Cons,              0x4001u);
  DEF_TRAITS_PTR(Reference,         0x4002u);

  // objects
  DEF_TRAITS_PTR_MATCH(ManagedType, 0x8000u);
  DEF_TRAITS_PTR(String,            0x8001u);
  DEF_TRAITS_PTR(Symbol,            0x8002u);
  DEF_TRAITS_PTR(BuiltinFunction,   0x8003u);
  DEF_TRAITS_PTR(Form,              0x8005u);

  DEF_TRAITS_PTR_MATCH(Container,   0xc000u);
  DEF_TRAITS_PTR(Array,             0xc001u);
  DEF_TRAITS_PTR(Function,          0xc002u);
  
  DEF_TRAITS_PTR_GT(Collectible,    0x0fffu);
}


#undef DEF_TRAITS_NUL
#undef DEF_TRAITS_NUL_MATCH
#undef DEF_TRAITS_INT
#undef DEF_TRAITS_PTR
#undef DEF_TRAITS_PTR_MATCH
#undef DEF_TRAITS_PTR_LT
#undef DEF_TRAITS_PTR_GT
#undef DEF_TRAITS_NUL_LT
#undef DEF_TRAITS_NUL_GT
#undef DEF_TRAITS_PCR_CHOIC
