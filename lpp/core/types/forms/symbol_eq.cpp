#include <lpp/core/types/forms/symbol_eq.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/cell.h>

using SymbolEq = Lisp::SymbolEq;
using Symbol = Lisp::Symbol;

SymbolEq::SymbolEq(Symbol * _symb, std::function<void()> func) : cb(func)
{
  cells.emplace_back(_symb);
  symb = _symb;
}

bool SymbolEq::isInstance(const Cell & cell) const
{
  return (cell.as<Symbol>() == symb);
}

bool SymbolEq::match(const Cell & cell) const
{
  if(cell.as<Symbol>() == symb)
  {
    if(cb)
    {
      cb();
    }
    return true;
  }
  else
  {
    return false;
  }
}
