#pragma once
#include <memory>
#include <lpp/core/types/form.h>

/////////////////////////////////////////////////////////////////
namespace Lisp
{
  namespace Scheme
  {
    class Scheme : public ::Lisp::Form::Form
    {
    public:
      Scheme();
      void init() override;
      bool isInstance(const Cell & cell) const override;
    private:
      Form * expression;
    };
  }
}
