#include <assert.h>
#include "lisp_cell.h"
#include "lisp_object.h"
#include "lisp_cons.h"

Lisp::Cell& Lisp::Cell::operator=(const Object & rhs)
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

Lisp::Cell::Cell(Cons * cons)
{
  typeId = Lisp::Cons::typeId;
  data.cons = cons;
}

