#pragma once
#include <lpp/core/types/type_id.h>

namespace Lisp
{
  class Vm;

  class FunctionInterface : public ManagedType
  {
  public:
    virtual ~FunctionInterface() {}
    virtual void eval(Vm& vm) const = 0;
  };
}
