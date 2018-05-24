#include "core/gc/garbage_collector.h"

using GarbageCollector = Lisp::GarbageCollector;
using Cell = Lisp::Cell;

GarbageCollector::GarbageCollector(std::size_t consPageSize,
                                   unsigned short _garbageSteps,
                                   unsigned short _recycleSteps)
  : consPages(consPageSize),
    conses({CollectibleContainer<Cons>(Lisp::Color::Void, this),
            CollectibleContainer<Cons>(Lisp::Color::White, this),
            CollectibleContainer<Cons>(Lisp::Color::Grey, this),
            CollectibleContainer<Cons>(Lisp::Color::Black, this),
            CollectibleContainer<Cons>(Lisp::Color::WhiteRoot, this),
            CollectibleContainer<Cons>(Lisp::Color::GreyRoot, this),
            CollectibleContainer<Cons>(Lisp::Color::BlackRoot, this) }),
    garbageSteps(_garbageSteps),
    recycleSteps(_recycleSteps),
    backGarbageSteps(_garbageSteps),
    backRecycleSteps(_recycleSteps)
{
}

std::vector<Cell> GarbageCollector::getCollectible(Color color) const
{
  std::vector<Cell> ret;
  if(color == Color::Free)
  {
    freeConses.addTo(ret);
  }
  else
  {
    conses[(unsigned char)color].addTo(ret);
  }
  return ret;
}

std::vector<Cell> GarbageCollector::getCollectible() const
{
  std::vector<Cell> ret;
}
