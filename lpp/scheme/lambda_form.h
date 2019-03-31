#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  class Form;
  class Cons;
  class Function;

  namespace Scheme
  {
    class LambdaForm : public Lisp::Form
    {
    public:
      LambdaForm(Form * _body);
      void init() override;
      bool isInstance(const Cell & cell) const;
    private:
      inline bool parse(Function * func, Cons *cdr) const;
      Form * lambdaSymbol;
      Form * argList;
      Form * body;
    };
  }
}
