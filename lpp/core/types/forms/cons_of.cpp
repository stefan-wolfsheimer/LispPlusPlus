#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/cons.h>

using Cons = Lisp::Cons;
using ConsOf = Lisp::Form::ConsOf;
using Form = Lisp::Form::Form;

ConsOf::ConsOf(const Cell & _car, const Cell & _cdr)
{
  /* @todo make exection */
  assert(_car.isA<Form>());
  assert(_cdr.isA<Form>());
  cells.push_back(_car);
  cells.push_back(_cdr);
  car = _car.as<Form>();
  cdr = _cdr.as<Form>();
}

bool ConsOf::isInstance(const Cell & cell) const
{
  auto cons = cell.as<Cons>();
  return
    cons && 
    car->isInstance(cons->getCarCell()) && 
    cdr->isInstance(cons->getCdrCell());
}
