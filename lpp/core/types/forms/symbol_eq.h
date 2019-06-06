#pragma once
#include <lpp/core/types/form.h>
#include <lpp/core/types/form_builder.h>

namespace Lisp
{
  class Symbol;

  template<typename BUILDER>
  class SymbolEq;

  template<>
  class SymbolEq<void> : public virtual Form
  {
  public:
    SymbolEq(Symbol *);
    bool isInstance(const Cell & cell) const override;
  private:
    const Symbol * symb;
  };

  template<typename BUILDER>
  class SymbolEq : public FormBuilder<BUILDER>,
                   public SymbolEq<void>
  {
  public:
    SymbolEq(Symbol * s,
             void(BUILDER::*_func)(const Cell & cell)=nullptr)
      : func(_func),
        SymbolEq<void>(s) {}

    bool match(const Cell & cell, BUILDER & builder) const override
    {
      if(SymbolEq<void>::isInstance(cell))
      {
        if(func)
        {
          ((builder).*func)(cell);
        }
        return true;
      }
      return false;
    }

  private:
    void(BUILDER::*func)(const Cell & cell);
  };

}
