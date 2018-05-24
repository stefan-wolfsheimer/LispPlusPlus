#pragma once
#include "lisp_type_id.h"

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
