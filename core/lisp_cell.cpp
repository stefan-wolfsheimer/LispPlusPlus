#include <assert.h>
#include "lisp_cell.h"
#include "lisp_object.h"
#include "types/cons.h"
#include "lisp_symbol_factory.h"

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

Cell::~Cell()
{
  unset();
}

void Lisp::Cell::init(Lisp::Cons * cons,
                      Lisp::TypeId _typeId)
{
  //todo replace Cons with Collectible
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
