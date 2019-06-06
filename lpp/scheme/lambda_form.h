#pragma once
#include <lpp/core/types/form_builder.h>
#include <lpp/scheme/builder.h>

namespace Lisp
{
  class Cell;

  namespace Scheme
  {
    class LambdaForm : public Lisp::FormBuilder<Builder>
    {
    public:
      LambdaForm(Lisp::FormBuilder<Builder> * _argList,
                 Lisp::FormBuilder<Builder> * _body);
      bool isInstance(const Cell & cell) const override;
      bool match(const Cell & cell, Builder & builder) const override;
    private:
      Lisp::FormBuilder<Builder> * argList;
      Lisp::FormBuilder<Builder> * body;
    };
  }
}
