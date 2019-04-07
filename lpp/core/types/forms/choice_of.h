#pragma once
#include <functional>
#include <lpp/core/types/form.h>

namespace Lisp
{
  class ChoiceOf : public Form
  {
  public:
    ChoiceOf(const std::vector<Form*> & forms=std::vector<Form*>(),
             std::function<void(Form *, const Cell & cell)> func=nullptr);
    void add(Form * f);
    bool isInstance(const Cell & cell) const override;
    bool match(const Cell & cell) const override;
  private:
    std::function<void(Form *, const Cell & cell)> cb;
  };
}
