#pragma once
#include <lpp/core/types/container.h>

namespace Lisp
{
  /**
   * Managed objects that can have garbage collected sub-cells.
   */
  class PolymorphicContainer : public Container
  {
  public:
    virtual TypeId getTypeId() const override
    {
      return TypeTraits<PolymorphicContainer>::getTypeId();
    }
  };
}
