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
#include <type_traits>
#include <lpp/core/types/collectible.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/lisp_object.h>

namespace Lisp
{
  class ConsFactory;
  template<typename T> class CollectibleContainer;

  class Cons : public Collectible
  {
  public:
    friend class ConsFactory;// todo: remove this friendship
    friend class ConsPages;
    friend class ConsContainer;
    friend class Object;
    friend class CollectibleContainer<Cons>;

    using Color = Lisp::Color;
    inline Object getCar() const;
    inline Object getCdr() const;
    inline const Cell & getCarCell() const;
    inline const Cell & getCdrCell() const;
    inline void unsetCar();
    inline void unsetCdr();
    inline void setCar(const Object & rhs);
    inline void setCdr(const Object & rhs);
    void setCar(Cons * cons,
                TypeId typeId=TypeTraits<Cons>::typeId);
    void setCdr(Cons * cons,
                TypeId typeId=TypeTraits<Cons>::typeId);
    inline void forEachChild(std::function<void(const Cell&)> func) const;
  private:
    Cell car;
    Cell cdr;
    void unroot();
    void root();
    Cons();
  };
}

////////////////////////////////////////////////////////////////////////////////
Lisp::Object Lisp::Cons::getCar() const
{
  return Lisp::Object(car);
}

Lisp::Object Lisp::Cons::getCdr() const
{
  return Lisp::Object(cdr);
}

const Lisp::Cell & Lisp::Cons::getCarCell() const
{
  return car;
}

const Lisp::Cell & Lisp::Cons::getCdrCell() const
{
  return cdr;
}

void Lisp::Cons::unsetCar()
{
  car = Lisp::nil;
}

void Lisp::Cons::unsetCdr()
{
  cdr = Lisp::nil;
}

void Lisp::Cons::setCar(const Object & rhs)
{
  Lisp::Cons * cons = rhs.as<Cons>();
  if(cons)
  {
    setCar(cons, rhs.typeId);
  }
  else
  {
    // set non-cons
    car = rhs;
  }
}

void Lisp::Cons::setCdr(const Object & rhs)
{
  Lisp::Cons * cons = rhs.as<Cons>();
  if(cons)
  {
    setCdr(cons, rhs.typeId);
  }
  else
  {
    // set non-cons
    cdr = rhs;
  }
}

void Lisp::Cons::forEachChild(std::function<void(const Cell&)> func) const
{
  func(getCarCell());
  func(getCdrCell());
}
