#pragma once
#include <functional>
#include <lpp/core/types/form.h>

namespace Lisp
{
  namespace Scheme
  {
    class FunctionEvalForm : public Lisp::Form
    {
    public:
      FunctionEvalForm(Form * _func,
                       Form * _expression,
                       std::function<void()> _funcPush,
                       std::function<void(std::size_t n, const Cell & f)> func);
      void init() override;
      bool isInstance(const Cell & cell) const override;
      bool match(const Cell & cell) const override;
    private:
      std::function<void()> cbPush;
      std::function<void(std::size_t n, const Cell & f)> cb;
      Form * expression;
      Form * function;
    };
  }
}
