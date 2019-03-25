#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  namespace Form
  {
    class ChoiceOf : public Form
    {
    public:
      ChoiceOf(const std::vector<Form*> &  _member);
      bool isInstance(const Cell & cell) const override;
    };
  }
}
