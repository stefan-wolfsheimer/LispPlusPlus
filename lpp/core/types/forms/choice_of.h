#pragma once
#include <functional>
#include <lpp/core/types/form.h>

namespace Lisp
{
  class ChoiceOf : public Form
  {
  public:
    ChoiceOf(const std::vector<Form*> &  _member, std::function<void(Form *, const Cell & cell)> func=nullptr);
    bool isInstance(const Cell & cell) const override;
  private:
    std::function<void(Form *, const Cell & cell)> cb;
  };
}
