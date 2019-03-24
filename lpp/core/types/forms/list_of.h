#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  //@todo refactor XForm -> Form
  namespace Form
  {
    class ListOf : public Form
    {
    public:
      ListOf(const Cell & _member);
      bool isInstance(const Cell & cell) const override;
    private:
      Form * member;
    };
  }
}
