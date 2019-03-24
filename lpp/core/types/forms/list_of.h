#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  namespace Form
  {
    class ListOf : public Form
    {
    public:
      ListOf(Form * _member);
      bool isInstance(const Cell & cell) const override;
    private:
      Form * member;
    };
  }
}
