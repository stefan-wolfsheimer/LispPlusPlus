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
#include <lpp/core/types/array.h>

namespace Lisp
{
  class GarbageCollector;
  template<typename T> class CollectibleContainer;

  /**
   * A Cons-like object that is managed by garbage collector.
   * References of conses are only counted from Object instances (not from Cell).
   * Conses are collected by the garbage collector through the reachability graph.
   * It is possible to derive (non-polymorphic) classes from Cons, but no additional
   * data members should be added.
   */
  class BasicCons : public Collectible,
                    public CollectibleMixin<BasicCons>
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
    inline void setCar(const Cell & rhs);
    inline void setCdr(const Cell & rhs);

    /**
     * Call function func for car and cons.
     */
    inline void forEachChild(std::function<void(const Cell&)> func) const;

    /** 
     * Move white car and cdr cell to grey set.
     * @return true
     */
    inline bool greyChildren();
    inline bool recycleNextChild();
  private:
    inline void setCarCdr(Cell & carcdr, BasicCons * cons, TypeId typeId);
    inline void setCarCdr(Cell & carcdr, Container * cons, TypeId typeId);
    inline void setCarCdr(Cell & carcdr, ManagedType * cons, TypeId typeId);

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
    BasicCons();
  };

  class Cons : public BasicCons
  {
  };
}

////////////////////////////////////////////////////////////////////////////////
inline Lisp::BasicCons::BasicCons() : car(Lisp::nil), cdr(Lisp::nil)
{
}

inline Lisp::TypeId Lisp::BasicCons::getTypeId() const
{
  return TypeTraits<Cons>::getTypeId();
}

inline Lisp::Object Lisp::BasicCons::getCar() const
{
  return Lisp::Object(car);
}

inline Lisp::Object Lisp::BasicCons::getCdr() const
{
  return Lisp::Object(cdr);
}

inline const Lisp::Cell & Lisp::BasicCons::getCarCell() const
{
  return car;
}

inline const Lisp::Cell & Lisp::BasicCons::getCdrCell() const
{
  return cdr;
}

inline void Lisp::BasicCons::unsetCar()
{
  car = Lisp::nil;
}

inline void Lisp::BasicCons::unsetCdr()
{
  cdr = Lisp::nil;
}

inline void Lisp::BasicCons::setCar(const Cell & rhs)
{
  if(rhs.isA<BasicCons>())
  {
    setCarCdr(car, rhs.data.pCons, rhs.getTypeId());
  }
  else if(rhs.isA<Container>())
  {
    setCarCdr(car, rhs.data.pContainer, rhs.getTypeId());
  }
  else if(rhs.isA<ManagedType>())
  {
    setCarCdr(car, rhs.data.pManaged, rhs.getTypeId());
  }
  else
  {
    car = rhs;
  }
}

inline void Lisp::BasicCons::setCdr(const Cell & rhs)
{
  if(rhs.isA<BasicCons>())
  {
    setCarCdr(cdr, rhs.data.pCons, rhs.getTypeId());
  }
  else if(rhs.isA<Container>())
  {
    setCarCdr(cdr, rhs.data.pContainer, rhs.getTypeId());
  }
  else if(rhs.isA<ManagedType>())
  {
    setCarCdr(cdr, rhs.data.pManaged, rhs.getTypeId());
  }
  else
  {
    cdr = rhs;
  }
}

inline void Lisp::BasicCons::forEachChild(std::function<void(const Cell&)> func) const
{
  func(car);
  func(cdr);
}

inline bool Lisp::BasicCons::greyChildren()
{
  car.grey();
  cdr.grey();
  return true;
}

inline bool Lisp::BasicCons::recycleNextChild()
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

inline void Lisp::BasicCons::gcStep()
{
  car.grey();
  cdr.grey();
  auto toContainer = getContainer()->getToContainer();
  if(toContainer)
  {
    getContainer()->remove(this);
    toContainer->add(this);
  }
}

inline void Lisp::BasicCons::setCarCdr(Cell & carcdr, BasicCons * cons, TypeId typeId)
{
  cons->grey();
  carcdr = Lisp::nil;
  carcdr.init(cons, typeId);
  gcStep();
}

inline void Lisp::BasicCons::setCarCdr(Cell & carcdr, Container * cons, TypeId typeId)
{
  cons->grey();
  carcdr = Lisp::nil;
  carcdr.init(cons, typeId);
  gcStep();
}

inline void Lisp::BasicCons::setCarCdr(Cell & carcdr, ManagedType * cons, TypeId typeId)
{
  carcdr = Lisp::nil;
  carcdr.init(cons, typeId);
  gcStep();
}
