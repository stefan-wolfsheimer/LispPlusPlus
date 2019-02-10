#include <lpp/core/compiler/list_pattern.h>
#include <lpp/core/cell.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/exception.h>

using ListPattern = Lisp::ListPattern;


ListPattern::ListPattern(std::shared_ptr<BasicType> _item) : item(_item)
{
}

std::shared_ptr<ListPattern> ListPattern::make(std::shared_ptr<BasicType> _item)
{
  return std::shared_ptr<ListPattern>(new ListPattern(_item));
}

bool ListPattern::isInstance(const Cell & cell)
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
      if(item)
      {
        if(!item->isInstance(cons->getCarCell()))
        {
          return false;
        }
        c = &cons->getCdrCell();
      }
    }
    else
    {
      return false;
    }
  }
}
