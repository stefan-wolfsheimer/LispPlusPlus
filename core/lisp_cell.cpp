#include <assert.h>
#include "lisp_cell.h"
#include "lisp_object.h"
#include "types/lisp_cons.h"
#include "types/lisp_managed_type.h"
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
    ((ManagedType*)(data.ptr))->refCount++;
  }
  return *this;
}

Lisp::Cell::Cell(const Cell & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<ManagedType>())
  {
    ((ManagedType*)(data.ptr))->refCount++;
  }
}

Lisp::Cell::Cell(const Object & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<ManagedType>())
  {
    ((ManagedType*)(data.ptr))->refCount++;
  }
}


Cell::~Cell()
{
  unset();
}

void Cell::unset()
{
  if(isManagedTypeId(typeId))
  {
    ManagedType * obj = (ManagedType*)data.ptr;
    assert(obj->refCount);
    if(! --obj->refCount)
    {
      delete obj;
      data.ptr = nullptr;
    }
  }
}
