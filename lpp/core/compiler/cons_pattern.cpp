#include <lpp/core/compiler/cons_pattern.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/cell.h>

using ConsPattern = Lisp::ConsPattern;

ConsPattern::ConsPattern(std::shared_ptr<BasicType> _car,
                         std::shared_ptr<BasicType> _cdr) : car(_car), cdr(_cdr)
{
}

std::shared_ptr<ConsPattern> ConsPattern::make(std::shared_ptr<BasicType> _car,
                                               std::shared_ptr<BasicType> _cdr)
{
  return std::shared_ptr<ConsPattern>(new ConsPattern(_car, _cdr));
}

bool ConsPattern::isInstance(const Cell & cell)
{
  auto cons = cell.as<Cons>();
  if(cons)
  {
    return
      (car ? car->isInstance(cons->getCarCell()) : true) && 
      (cdr ? cdr->isInstance(cons->getCdrCell()) : true);
  }
  else
  {
    return false;
  }
  return isInstanceStatic(cell);
}

