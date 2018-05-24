#include <vector>
#include <lpp/core/gc/cons_pages.h>
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/gc/unmanaged_collectible_container.h>

#define CONS_PAGE_SIZE 512

namespace Lisp
{
  class GarbageCollector
  {
  public:
    GarbageCollector(std::size_t consPageSize,
                     unsigned short _garbageSteps=1,
                     unsigned short _recycleSteps=1);
    std::vector<Cell> getCollectible() const;
    std::vector<Cell> getCollectible(Color color) const;
    std::vector<Cell> getRootCollectible() const;
    std::vector<Cell> getReachable() const;

  protected: //todo make private
    CollectibleContainer<Cons> conses[7];
    UnmanagedCollectibleContainer<Cons> freeConses;

    ConsPages consPages;
    unsigned short int garbageSteps;
    unsigned short int recycleSteps;
    unsigned short int backGarbageSteps;
    unsigned short int backRecycleSteps;
  };
}
