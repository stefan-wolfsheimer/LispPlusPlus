#include <lpp/core/types/lisp_type_id.h>
#include <lpp/core/gc/color.h>

namespace Lisp
{
  class Collectible : public BasicType
  {
  protected:
    // todo reduce memory footprint
    // reference to color vector
    ConsFactory * consFactory;
    Color color;
    std::size_t refCount;
    std::size_t index;
  };
}
