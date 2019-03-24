#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/cons.h>

using Cons = Lisp::Cons;
using ConsOf = Lisp::Form::ConsOf;
using Form = Lisp::Form::Form;

ConsOf::ConsOf(Form * _car, Form * _cdr)
{
  cells.push_back(Cell(_car));
  cells.push_back(Cell(_cdr));
  car = _car;
  cdr = _cdr;
}

bool ConsOf::isInstance(const Cell & cell) const
{
  auto cons = cell.as<Cons>();
  return
    cons && 
    car->isInstance(cons->getCarCell()) && 
    cdr->isInstance(cons->getCdrCell());
}
