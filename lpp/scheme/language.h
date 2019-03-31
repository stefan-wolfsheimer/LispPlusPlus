#pragma once
#include <memory>
#include <lpp/core/types/form.h>

/////////////////////////////////////////////////////////////////
namespace Lisp
{
  class Allocator;
  class Vm;

  namespace Scheme
  {
    class Language : public ::Lisp::Form
    {
    public:
      Language();
      void init() override;
      bool isInstance(const Cell & cell) const override;
      Object compile(const Cell & cell) const;
      Object compileAndEval(Vm & vm, const Cell & cell) const;
    private:
      Form * expression;
      Form * topLevelForm;
      Form * lambdaForm;
    };
  }
}
