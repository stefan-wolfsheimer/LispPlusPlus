#pragma once
#include <lpp/core/types/managed_type.h>
#include <lpp/core/object.h>

namespace Lisp
{
  class Cell;
  class Jit;

  class Form : public ManagedType
  {
  public:
    virtual ~Form(){}
    virtual void compile(Jit & jit, Function *, const Cell & obj) const=0;
  };
}
