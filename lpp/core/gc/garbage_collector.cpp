#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/collectible.h>

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
    backRecycleSteps(_recycleSteps),
    fromColor(Color::White),
    toColor(Color::Black),
    fromRootColor(Color::WhiteRoot),
    toRootColor(Color::BlackRoot)
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

bool GarbageCollector::checkSanity(Color color) const
{
  std::size_t i = 0;
  auto cells = getCollectible(color);
  for(const Cell & cell : cells)
  {
    if(!cell.isA<Collectible>())
    {
      return false;
    }
    auto coll = cell.as<Collectible>();
    if(coll->getColor() != color)
    {
      return false;
    }
    if(coll->getIndex() != i)
    {
      return false;
    }
    i++;
  }
  if(color == getToColor())
  {
    for(const Cell & cell : cells)
    {
      // todo: implement iterator for collectible and use Collectible
      auto cons = cell.as<Cons>();
      if(cons->getCarCell().isA<const Cons>())
      {
        if(cons->getCarCell().as<const Cons>()->getColor() == getFromColor())
        {
          return false;
        }
      }
      if(cons->getCdrCell().isA<const Cons>())
      {
        if(cons->getCdrCell().as<const Cons>()->getColor() == getFromColor())
        {
          return false;
        }
      }
    }
  }
  return true;
}

bool GarbageCollector::checkSanity() const
{
  return
    checkSanity(Color::White) ||
    checkSanity(Color::Grey) ||
    checkSanity(Color::Black) ||
    checkSanity(Color::WhiteRoot) ||
    checkSanity(Color::GreyRoot) ||
    checkSanity(Color::BlackRoot);
}
