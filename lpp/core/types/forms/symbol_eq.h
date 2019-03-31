#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  class Symbol;

  class SymbolEq : public Form
  {
  public:
    SymbolEq(Symbol *, std::function<void()> func=nullptr);
    bool isInstance(const Cell & cell) const override;
  private:
    std::function<void()> cb;
    const Symbol * symb;
  };
}
