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

namespace Lisp
{
  namespace Details
  {
    template<typename T>
    struct Converter;
  }
  class Cons;
  class Object;

  class Cell
  {
  public:
    template<typename T>
    friend class Lisp::Details::Converter;
    friend class Lisp::Cons;

    Cell(const Object & rhs);
    Cell(Cons * cons);
    Cell& operator=(const Object & rhs);

    inline std::size_t getTypeId() const;

    template<typename T>
    inline bool isA() const;

    template<typename T>
    inline T * as() const;

  protected:
    Cell(std::size_t _typeId) : typeId(_typeId) {}
    std::size_t typeId;
    union
    {
      Cons * cons;
    } data;
  };
}

namespace Lisp
{
  namespace Details
  {
    template<typename T>
    struct Converter
    {
      static T * as(const Lisp::Cell * obj)
      {
        return nullptr;
      }
    };
  }
}

inline std::size_t Lisp::Cell::getTypeId() const
{
  return typeId;
}

template<typename T>
inline bool Lisp::Cell::isA() const
{
  return typeId == T::typeId;
}

template<typename T>
inline T * Lisp::Cell::as() const
{
  return Lisp::Details::Converter<T>::as(this);
}
