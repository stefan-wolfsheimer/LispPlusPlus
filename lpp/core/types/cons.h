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
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/types/collectible.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/object.h>

namespace Lisp
{
  class GarbageCollector;
  template<typename T> class CollectibleContainer;

  class Cons : public Collectible,
               public CollectibleMixin<Cons>
  {
  public:
    friend class GarbageCollector;
    friend class ConsPages;
    friend class Object;

    using Color = Lisp::Color;
    inline TypeId getTypeId() const;
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
    inline void forEachChild(std::function<void(const Cell&)> func) const;
    inline bool greyChildren();
    inline bool unsetNonCollectibleChildren();
  private:
    Cell car;
    Cell cdr;
    /**
     * Performs a garbage collector step on cons
     *
     * If color is getFromColor() then change color to getToColor()
     * If color is getFromRootColor() then change color to getToRootColor()
     * If color is Color::Grey then  change color to getToColor()
     * If color is Color::GreyRoot then change color to getToRootColor()
     * All children having getFromColor() or getFromRootColor()) are changed
     * to Color::GreyRoot or Color::Grey.
     */
    inline void gcStep();
    Cons();
  };
}

////////////////////////////////////////////////////////////////////////////////
inline Lisp::Cons::Cons() : car(Lisp::nil), cdr(Lisp::nil)
{
}

inline Lisp::TypeId Lisp::Cons::getTypeId() const
{
  return TypeTraits<Cons>::typeId;
}

inline Lisp::Object Lisp::Cons::getCar() const
{
  return Lisp::Object(car);
}

inline Lisp::Object Lisp::Cons::getCdr() const
{
  return Lisp::Object(cdr);
}

inline const Lisp::Cell & Lisp::Cons::getCarCell() const
{
  return car;
}

inline const Lisp::Cell & Lisp::Cons::getCdrCell() const
{
  return cdr;
}

inline void Lisp::Cons::unsetCar()
{
  car = Lisp::nil;
}

inline void Lisp::Cons::unsetCdr()
{
  cdr = Lisp::nil;
}

inline void Lisp::Cons::setCar(const Object & rhs)
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

inline void Lisp::Cons::setCdr(const Object & rhs)
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

inline void Lisp::Cons::setCar(Cons * cons, TypeId _typeId)
{
  car = Lisp::nil;
  car.typeId = _typeId; 
  car.data.ptr = cons;
  gcStep();
}

inline void Lisp::Cons::setCdr(Cons * cons, TypeId _typeId)
{
  cdr = Lisp::nil;
  cdr.typeId = _typeId;
  cdr.data.ptr = cons;
  gcStep();
}

inline void Lisp::Cons::forEachChild(std::function<void(const Cell&)> func) const
{
  func(car);
  func(cdr);
}

inline bool Lisp::Cons::greyChildren()
{
  getCarCell().grey();
  getCdrCell().grey();
  return true;
}

inline bool Lisp::Cons::unsetNonCollectibleChildren()
{
  if(!car.isA<Collectible>())
  {
    unsetCar();
  }
  if(!cdr.isA<Collectible>())
  {
    unsetCdr();
  }
  return true;
}

inline void Lisp::Cons::gcStep()
{
  getCarCell().grey();
  getCdrCell().grey();
  auto toContainer = getContainer()->getToContainer();
  if(toContainer)
  {
    getContainer()->remove(this);
    toContainer->add(this);
  }
}
