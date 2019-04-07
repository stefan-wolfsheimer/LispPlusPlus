#pragma once
#include <functional>
#include <lpp/core/types/form.h>

namespace Lisp
{
  namespace Scheme
  {
    /**
     * Procedure call
     *
     * r6rs, 9.1
     * Note: In contrast to other dialects of Lisp, the order of evalua-
     * tion is unspecified, and the operator expression and the operand
     * expressions are always evaluated with the same evaluation rules.
     */
    class ProcedureCallForm : public Lisp::Form
    {
    public:
      ProcedureCallForm(Form * _expression,
                        std::function<void()> _funcPush,
                        std::function<void(std::size_t n)> func);
      void init() override;
      bool isInstance(const Cell & cell) const override;
      bool match(const Cell & cell) const override;
    private:
      std::function<void()> cbPush;
      std::function<void(std::size_t n)> cb;
      Form * expression;
    };
  }
}
