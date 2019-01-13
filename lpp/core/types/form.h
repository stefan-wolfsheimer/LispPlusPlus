#pragma once
#include <lpp/core/types/managed_type.h>

namespace Lisp
{
  class Cell;
  class Jit;

  class Form : public ManagedType
  {
  public:
    virtual ~Form(){}
    virtual void pass1(Jit & jit, const Cell & cdr) const = 0;
    virtual void pass2(Jit & jit, const Cell & cdr) const = 0;
    virtual void exception(Jit & jit, const Cell & cdr) const = 0;
  };
}
