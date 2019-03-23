#pragma once
#include <lpp/core/types/managed_type.h>

namespace Lisp
{
  class Cell;

  /**
   * Meta type
   */
  class BasicType : public ManagedType
  {
  public:
    virtual ~BasicType() {}
    // @todo move virtual method to form matcher
    virtual bool isInstance(const Cell & cell) = 0;
  };

}
