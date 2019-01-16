#pragma
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
    virtual bool isInstance(const Cell & cell) = 0;
  };

}
