#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/cons.h>

using Cons = Lisp::Cons;
using ConsOfForm = Lisp::ConsOf<void>;
using Form = Lisp::Form;

ConsOfForm::ConsOf(Form * _car, Form * _cdr, std::function<void(Form *, const Cell & car, Form *, const Cell & cdr)> func)
  : cb(func)
{
  cells.push_back(Cell(_car));
  cells.push_back(Cell(_cdr));
  car = _car;
  cdr = _cdr;
}

bool ConsOfForm::isInstance(const Cell & cell) const
{
  auto cons = cell.as<Cons>();
  return (cons &&
          car->isInstance(cons->getCarCell()) &&
          cdr->isInstance(cons->getCdrCell()));
}

bool ConsOfForm::match(const Cell & cell) const
{
  auto cons = cell.as<Cons>();
  if(cons &&
     car->match(cons->getCarCell()) &&
     cdr->match(cons->getCdrCell()))
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
