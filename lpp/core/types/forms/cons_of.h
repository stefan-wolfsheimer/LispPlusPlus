#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  namespace Form
  {
    class ConsOf : public Form
    {
    public:
      ConsOf(Form * _car, Form * _cdr);
      bool isInstance(const Cell & cell) const override;
    private:
      Form * car;
      Form * cdr;
    };
  }
}
