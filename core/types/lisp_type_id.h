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

namespace Lisp
{
typedef ::std::uint_least16_t TypeId;
inline bool isCoreTypeId(TypeId typeId);
inline bool isManagedTypeId(TypeId typeId);
inline bool isAtomTypeId(TypeId typeId);
inline bool isCoreTypeId(TypeId typeId);
inline bool isAtomTypeId(TypeId typeId);
inline bool isManagedTypeId(TypeId typeId);

}

inline bool Lisp::isCoreTypeId(Lisp::TypeId typeId)
{
  // largest bit flags managed types
  // 00: core classes, like cons, nil, symbol, lambda
  // 01: atoms (0x4000)
  // 10: managed types (0x8000)
  // 
  return !(typeId ^ 0xc000);
}

inline bool Lisp::isAtomTypeId(Lisp::TypeId typeId)
{
  return typeId & 0x4000;
}

inline bool Lisp::isManagedTypeId(Lisp::TypeId typeId)
{
  return typeId & 0x8000;
}


