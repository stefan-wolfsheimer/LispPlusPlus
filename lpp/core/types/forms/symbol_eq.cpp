#include <lpp/core/types/forms/symbol_eq.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/cell.h>

using SymbolEq = Lisp::SymbolEq<void>;
using Symbol = Lisp::Symbol;


SymbolEq::SymbolEq(Symbol * _symb)
{
  cells.emplace_back(_symb);
  symb = _symb;
}

bool SymbolEq::isInstance(const Cell & cell) const
{
  return (cell.as<Symbol>() == symb);
}
