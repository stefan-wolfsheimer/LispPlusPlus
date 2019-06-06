#pragma once
#include <lpp/core/types/form_builder.h>
#include <lpp/scheme/builder.h>

namespace Lisp
{
  class Cell;

  namespace Scheme
  {
    class ProcedureCallArgDecorator : public Lisp::FormBuilder<Builder>
    {
    public:
      ProcedureCallArgDecorator(Lisp::FormBuilder<Builder> * _expression);
      bool isInstance(const Cell & cell) const;
      bool match(const Cell & cell, Builder & builder) const;
    private:
      Lisp::FormBuilder<Builder> * expression;
    };
  }
}

