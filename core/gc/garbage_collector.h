#include <core/gc/cons_pages.h>

#define CONS_PAGE_SIZE 512

namespace Lisp
{
  class GarbageCollector
  {
  public:
    GarbageCollector(std::size_t consPageSize,
                     unsigned short _garbageSteps=1,
                     unsigned short _recycleSteps=1);
  protected: //todo make private
    ConsPages consPages;
    unsigned short int garbageSteps;
    unsigned short int recycleSteps;
    unsigned short int backGarbageSteps;
    unsigned short int backRecycleSteps;
  };
}
