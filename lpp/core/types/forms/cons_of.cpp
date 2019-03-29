#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/cons.h>

using Cons = Lisp::Cons;
using ConsOf = Lisp::ConsOf;
using Form = Lisp::Form;

ConsOf::ConsOf(Form * _car, Form * _cdr, std::function<void(Form *, const Cell & car, Form *, const Cell & cdr)> func)
  : cb(func)
{
  cells.push_back(Cell(_car));
  cells.push_back(Cell(_cdr));
  car = _car;
  cdr = _cdr;
}

bool ConsOf::isInstance(const Cell & cell) const
{
  auto cons = cell.as<Cons>();
  if(cons &&
     car->isInstance(cons->getCarCell()) &&
     cdr->isInstance(cons->getCdrCell()))
  {
    if(cb)
    {
      cb(car, cons->getCarCell(), cdr, cons->getCdrCell());
    }
    return true;
  }
  else
  {
    return false;
  }
}
