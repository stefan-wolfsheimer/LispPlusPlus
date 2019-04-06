#include <lpp/core/types/forms/list_of.h>
#include <lpp/core/types/cons.h>

using ListOf = Lisp::ListOf;
using Form = Lisp::Form;
using Cons = Lisp::Cons;
using Nil = Lisp::Nil;
using Cell = Lisp::Cell;

ListOf::ListOf(Form * _member, std::function<void(const Cell & car)> func) : member(_member), cb(func)
{
  cells.push_back(_member);
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
    if(cons && member->isInstance(cons->getCarCell()))
    {
      c = &cons->getCdrCell();
    }
    else
    {
      return false;
    }
  }
  return false;
}

bool ListOf::match(const Cell & cell) const
{
  const Cell * c = &cell;
  while(true)
  {
    if(c->isA<Nil>())
    {
      if(cb)
      {
        cb(cell);
      }
      return true;
    }
    Cons * cons = c->as<Cons>();
    if(cons)
    {
      if(!member->match(cons->getCarCell()))
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
