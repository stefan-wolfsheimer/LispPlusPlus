#include <assert.h>
#include "cell.h"
#include "object.h"
#include "types/cons.h"
#include "types/container.h"
#include "symbol_factory.h"

using Cell = Lisp::Cell;
using Cons = Lisp::Cons;
using Symbol = Lisp::Symbol;
using ManagedType = Lisp::ManagedType;

Cell& Cell::operator=(const Object & rhs)
{
  assert(!rhs.isA<Cons>() || rhs.as<Cons>()->isRoot());
  unset();
  typeId = rhs.typeId;
  data = rhs.data;
  if(isA<ManagedType>())
  {
    static_cast<ManagedType*>(data.ptr)->refCount++;
  }
  return *this;
}


Lisp::Cell::Cell(const Cell & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<ManagedType>())
  {
    static_cast<ManagedType*>(data.ptr)->refCount++;
  }
}

Lisp::Cell::Cell(const Object & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<ManagedType>())
  {
    static_cast<ManagedType*>(data.ptr)->refCount++;
  }
}

Cell::Cell(Container * rhs, TypeId typeId)
{
  typeId = typeId;
  data.ptr = rhs;
}

Cell::~Cell()
{
  unset();
}

void Lisp::Cell::init(Lisp::Collectible * cons,
                      Lisp::TypeId _typeId)
{
  typeId = _typeId;
  data.ptr = static_cast<BasicType*>(cons);
}

void Cell::unset()
{
  if(TypeTraits<ManagedType>::isA(typeId))
  {
    ManagedType * obj = static_cast<ManagedType*>(data.ptr);
    assert(obj->refCount);
    if(! --obj->refCount)
    {
      delete obj;
      data.ptr = nullptr;
    }
  }
}

void Cell::forEachChild(std::function<void(const Cell&)> func) const
{
  if(isA<Cons>())
  {
    as<Cons>()->forEachChild(func);
  }
}

void Cell::grey() const
{
  auto cons = as<Cons>();
  if(cons)
  {
    cons->grey();
    return;
  }
  auto container = as<Container>();
  if(container)
  {
    container->grey();
    return;
  }
}


