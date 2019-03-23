#pragma once
#include <lpp/core/types/managed_type.h>

namespace Lisp
{
  /**
   * Base class of user defined Objects
   */
  class PolymorphicObject : public ManagedType
  {
  public:
    virtual ~PolymorphicObject() {}
  };
}
