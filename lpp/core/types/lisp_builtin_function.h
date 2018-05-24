#pragma once
#include <functional>
#include <lpp/core/types/lisp_function_interface.h>

namespace Lisp
{
  class BuiltinFunction : public FunctionInterface
  {
  public:
    typedef std::function<void(Vm&)> FunctionType;
    BuiltinFunction(FunctionType lambda) {
    }

    virtual void eval(Vm& vm) const override {
      // @todo match lambda list, throw exception if no match
    }
  };
}
