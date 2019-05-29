#pragma once
#include <lpp/core/types/form.h>
namespace Lisp
{
  template<typename BUILDER>
  class FormBuilder : public virtual Form
  {
  public:
    virtual bool match(const Cell & cell, BUILDER & obj) const = 0;
  };
}
