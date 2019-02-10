#pragma once
#include <lpp/core/types/cons.h>

namespace Lisp
{
  class Reference : public BasicCons
  {
  public:
    void setValue(const Cell & rhs);
    const Cell & getValue() const;
  };
}
