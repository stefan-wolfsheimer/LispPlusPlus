#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  //@todo refactor XForm -> Form
  namespace Form
  {
    class ConsOf : public Form
    {
    public:
      ConsOf(const Cell & _car, const Cell & _cdr);
      bool isInstance(const Cell & cell) const override;
    private:
      Form * car;
      Form * cdr;
    };
  }
}
