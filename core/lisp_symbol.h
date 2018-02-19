/******************************************************************************
Copyright (c) 2017-2018, Stefan Wolfsheimer

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
#include "lisp_object.h"

namespace Lisp
{
  class SymbolFactory;

  class Symbol
  {
  public:
    static const std::size_t typeId;
    inline std::size_t getRefCount() const;
    inline const std::string& getName() const;
  private:
    friend class SymbolFactory;
    friend class Cell;
    Symbol(const std::string & _name, SymbolFactory * _factory=nullptr, std::size_t _refCount=1);
    std::string name;
    std::size_t refCount;
    SymbolFactory * factory;
  };
}

inline Lisp::Symbol::Symbol(const std::string & _name, SymbolFactory * _factory, std::size_t _refCount)
  : name(_name), refCount(_refCount), factory(_factory)
{
}

inline std::size_t Lisp::Symbol::getRefCount() const
{
  return refCount;
}

inline const std::string& Lisp::Symbol::getName() const
{
  return name;
}

namespace Lisp
{
  namespace Details
  {
    template<>
    struct Converter<::Lisp::Symbol>
    {
      static Symbol * as(const Cell * obj)
      {
        if(obj->isA<Symbol>())
        {
          return obj->data.symbol;
        }
        else
        {
          return nullptr;
        }
      }
    };

    template<>
    struct Converter<const Symbol>
    {
      static const Symbol * as(const Cell * obj)
      {
        if(obj->isA<const Symbol>())
        {
          return obj->data.symbol;
        }
        else
        {
          return nullptr;
        }
      }
    };
  }
}

