#include <assert.h>
#include "lisp_cell.h"
#include "lisp_object.h"
#include "lisp_cons.h"
#include "lisp_symbol.h"

using Cell = Lisp::Cell;
using Cons = Lisp::Cons;
using Symbol = Lisp::Symbol;

Cell& Cell::operator=(const Object & rhs)
{
  assert(!rhs.isA<Cons>() || rhs.as<Cons>()->isRoot());
  typeId = rhs.typeId;
  data = rhs.data;
  return *this;
}

Lisp::Cell::Cell(const Object & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
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
}
