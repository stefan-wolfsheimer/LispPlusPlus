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
#include "lisp_type_id.h"
#include "core/lisp_object.h"
#include "core/lisp_cons_factory.h"

namespace Lisp
{
  template<typename T> class CollectibleContainer;
  class Cons : public BasicType
  {
    // todo: derive from GarbageCollectable
  public:
    friend class ConsFactory;
    friend class ConsContainer;
    friend class Object;
    friend class CollectibleContainer<Cons>;

    using Color = ConsFactory::Color;
    inline std::size_t getRefCount() const;
    inline Color getColor() const;
    inline bool isRoot() const;
    inline Object getCar() const;
    inline Object getCdr() const;
    inline const Cell & getCarCell() const;
    inline const Cell & getCdrCell() const;
    inline void unsetCar();
    inline void unsetCdr();
    inline void setCar(const Object & rhs);
    inline void setCdr(const Object & rhs);
    inline void setCar(Cons * cons,
                       TypeId typeId=TypeTraits<Cons>::typeId);
    inline void setCdr(Cons * cons,
                       TypeId typeId=TypeTraits<Cons>::typeId);
    inline std::size_t getIndex() const;
  private:
    // todo reduce memory footprint
    // reference to color vector
    ConsFactory * consFactory;
    Color color;
    std::size_t refCount;
    std::size_t index;
    Cell car;
    Cell cdr;
    inline void unroot();
    inline void root();
    Cons();
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
std::size_t Lisp::Cons::getRefCount() const
{
  return refCount;
}

Lisp::Cons::Color Lisp::Cons::getColor() const
{
  return color;
}

bool Lisp::Cons::isRoot() const
{
  return
    color == Cons::Color::WhiteRoot ||
    color == Cons::Color::BlackRoot ||
    color == Cons::Color::GreyRoot;
}

std::size_t Lisp::Cons::getIndex() const
{
  return index;
}

void Lisp::Cons::unroot()
{
  if(!--refCount)
  {
    consFactory->unroot(this);
  }
}

void Lisp::Cons::root()
{
  if(isRoot())
  {
    ++refCount;
  }
  else
  {
    consFactory->root(this);
  }
}

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

void Lisp::Cons::setCar(Cons * cons, TypeId _typeId)
{
  car = Lisp::nil;
  car.typeId = _typeId; 
  car.data.ptr = cons;
  consFactory->gcStep(this);
}

void Lisp::Cons::setCdr(Cons * cons, TypeId _typeId)
{
  cdr = Lisp::nil;
  cdr.typeId = _typeId;
  cdr.data.ptr = cons;
  consFactory->gcStep(this);
}
