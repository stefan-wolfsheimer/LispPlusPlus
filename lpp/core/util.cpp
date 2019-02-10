#include <lpp/core/util.h>
#include <lpp/core/exception.h>
#include <lpp/core/types/cons.h>

using Cell = Lisp::Cell;

bool Lisp::isAList(const Cell & cell)
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
      c = &cons->getCdrCell();
    }
    else
    {
      return false;
    }
  }
}

std::size_t Lisp::listLength(const Cell & cell)
{
  std::size_t ret = 0;
  forEachCar(cell, [&ret](const Cell &){ ret++; });
  return ret;
}

void Lisp::forEachCar(const Cell & cell,
                      std::function<void(const Cell&)> func)
{
  std::size_t ret = 0;
  const Cell * c = &cell;
  while(true)
  {
    if(c->isA<Nil>())
    {
      return;
    }
    Cons * cons = c->as<Cons>();
    if(cons)
    {
      func(cons->getCarCell());
      c = &cons->getCdrCell();
    }
    else
    {
      throw NotAList(cell);
    }
  }
}

