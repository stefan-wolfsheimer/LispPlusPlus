#include <lpp/core/types/forms/list_of.h>
#include <lpp/core/types/cons.h>

using ListOf = Lisp::Form::ListOf;
using Form = Lisp::Form::Form;
using Cons = Lisp::Cons;
using Nil = Lisp::Nil;
using Cell = Lisp::Cell;

ListOf::ListOf(const Cell & _member)
{
  /* @todo make exection */
  assert(_member.isA<Form>());
  cells.push_back(_member);
  member = _member.as<Form>();
}

bool ListOf::isInstance(const Cell & cell) const
{
  const Cell * c = &cell;
  while(true)
  {
    if(c->isA<Nil>())
    {
      return true;
    }
    Cons * cons = c->as<Cons>();
    if(cons)
    {
      if(!member->isInstance(cons->getCarCell()))
      {
        return false;
      }
      c = &cons->getCdrCell();
    }
    else
    {
      return false;
    }
  }
}
