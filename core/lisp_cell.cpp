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
  if(isA<Symbol>())
  {
    ((Lisp::Symbol*)(data.ptr))->refCount++;
  }
}

Lisp::Cell::Cell(const Object & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
  if(isA<Symbol>())
  {
    ((Lisp::Symbol*)(data.ptr))->refCount++;
  }
}


Cell::Cell(Cons * cons)
{
  typeId = Cons::typeId;
  data.ptr = cons;
}

Cell::Cell(Symbol * symbol)
{
  typeId = Symbol::typeId;
  symbol->refCount++;
  data.ptr = symbol;
}

Cell::~Cell()
{
  unset();
}

void Cell::unset()
{
  if(isA<Symbol>())
  {
    Symbol * symbol = (Symbol*)data.ptr;
    if(symbol->refCount)
    {
      symbol->refCount--;
    }
    if(symbol->refCount == 0)
    {
      symbol->factory->remove(symbol);
      symbol = nullptr;
    }
  }
}
