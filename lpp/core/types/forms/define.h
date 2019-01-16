#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  class BasicType;

  class Define : public Form
  {
  public:

    Define();

    virtual void pass1(Jit & jit, const Cell & cdr) const override;
    virtual void pass2(Jit & jit, const Cell & cdr) const override;
    virtual void exception(Jit & jit, const Cell & cdr) const override;

  private:
    std::shared_ptr<BasicType> pattern;

  };
}
