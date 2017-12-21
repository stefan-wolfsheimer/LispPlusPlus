#include <assert.h>
#include "lisp_cell.h"
#include "lisp_object.h"
#include "lisp_cons.h"

Lisp::Cell& Lisp::Cell::operator=(const Object & rhs)
{
  assert(!rhs.isA<Cons>() || rhs.as<Cons>()->getColor() == Cons::Color::Root);
  typeId = rhs.typeId;
  data = rhs.data;
  return *this;
}

Lisp::Cell::Cell(const Object & rhs)
{
  typeId = rhs.typeId;
  data = rhs.data;
}

