#include <assert.h>
#include "lisp_cell.h"
#include "lisp_object.h"
#include "types/lisp_cons.h"
#include "types/lisp_symbol.h"
#include "lisp_symbol_factory.h"

using Cell = Lisp::Cell;
using Cons = Lisp::Cons;
using Symbol = Lisp::Symbol;

Cell& Cell::operator=(const Object & rhs)
{
  assert(!rhs.isA<Cons>() || rhs.as<Cons>()->isRoot());
  unset();
  typeId = rhs.typeId;
  data = rhs.data;
  return *this;
}

Lisp::Cell::Cell(const Cell & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<Lisp::Symbol>())
  {
    data.symbol->refCount++;
  }
}

Lisp::Cell::Cell(const Object & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(rhs.isA<Lisp::Symbol>())
  {
    data.symbol->refCount++;
  }
}


Cell::Cell(Cons * cons)
{
  typeId = Cons::typeId;
  data.cons = cons;
}

Cell::Cell(Symbol * symbol)
{
  typeId = Symbol::typeId;
  data.symbol = symbol;
  data.symbol->refCount++;
}

Cell::~Cell()
{
  unset();
}

void Cell::unset()
{
  if(isA<Symbol>())
  {
    if(data.symbol->refCount)
    {
      data.symbol->refCount--;
    }
    if(data.symbol->refCount == 0)
    {
      data.symbol->factory->remove(data.symbol);
      data.symbol = nullptr;
    }
  }
}
