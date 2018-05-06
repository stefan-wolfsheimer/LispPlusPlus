#include <core/gc/cons_pages.h>

namespace Lisp
{
  class GarbageCollector
  {
  public:
    GarbageCollector(std::size_t consPageSize);
  protected: //todo make private
    ConsPages consPages;
  };
}
