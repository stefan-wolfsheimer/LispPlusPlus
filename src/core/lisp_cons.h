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
#include "lisp_object.h"
#include "lisp_i_cons_factory.h"

namespace Lisp
{
  class Cons
  {
  public:
    friend class ConsFactory;
    friend class Object;
    using Color = IConsFactory::Color;
    static const std::size_t typeId;
    inline std::size_t getRefCount() const;
    inline Color getColor() const;
    inline void unsetCar();
    inline void unsetCdr();
  private:
    IConsFactory * consFactory;
    unsigned char colorIndex;
    std::size_t refCount;
    Object car;
    Object cdr;
    inline void unroot();
    Cons();
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
std::size_t Lisp::Cons::getRefCount() const
{
  if(getColor() == Color::Root)
  {
    return refCount;
  }
  else
  {
    return 0u;
  }
}

Lisp::Cons::Color Lisp::Cons::getColor() const
{
  return consFactory->encodeColor(colorIndex);
}

void Lisp::Cons::unroot()
{
  if(!--refCount)
  {
    consFactory->unroot(this);
  }
}

void Lisp::Cons::unsetCar()
{
  car = Lisp::nil;
}

void Lisp::Cons::unsetCdr()
{
  cdr = Lisp::nil;
}



namespace Lisp
{
  namespace Details
  {
    template<>
    struct Converter<Cons>
    {
      static Cons * as(const Object * obj)
      {
        if(obj->isA<Cons>())
        {
          return obj->data.cons;
        }
        else
        {
          return nullptr;
        }
      }
    };
  }
}

