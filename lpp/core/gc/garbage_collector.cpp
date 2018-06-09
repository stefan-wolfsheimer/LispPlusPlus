/******************************************************************************
Copyright (c) 2018, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/
#include <stdexcept>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/collectible.h>

using GarbageCollector = Lisp::GarbageCollector;
using CollectibleGraph = Lisp::CollectibleGraph;

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

void GarbageCollector::forEachCons(const CollectibleContainer<Cons> & conses,
                                   std::function<void(const Cell &)> func) const
{
  for(auto itr = conses.cbegin(); itr != conses.cend(); ++itr)
  {
    func(*itr);
  }
}


void GarbageCollector::forEachCollectible(std::function<void(const Cell &)> func) const
{
  forEachCollectible(Color::WhiteRoot, func);
  forEachCollectible(Color::BlackRoot, func);
  forEachCollectible(Color::GreyRoot, func);
  forEachCollectible(Color::White, func);
  forEachCollectible(Color::Black, func);
  forEachCollectible(Color::Grey, func);
}

void GarbageCollector::forEachCollectible(Color color, std::function<void(const Cell &)> func) const
{
  forEachCons(conses[(unsigned char)color], func);
  //todo: other collectible
}

void GarbageCollector::forEachDisposedCollectible(std::function<void(const Cell &)> func) const
{
}

void GarbageCollector::forEachRootCollectible(std::function<void(const Cell &)> func) const
{
  forEachCollectible(Color::WhiteRoot, func);
  forEachCollectible(Color::BlackRoot, func);
  forEachCollectible(Color::GreyRoot, func);
}

void GarbageCollector::forEachReachable(std::function<void(const Cell &)> func) const
{
  std::unordered_set<Cell> todo;
  std::unordered_set<Cell> root;
  forEachRootCollectible([&todo](const Cell & cell){ todo.insert(cell); });
  while(!todo.empty())
  {
    const Cell cell = *todo.begin();
    func(cell);
    todo.erase(cell);
    root.insert(cell);
    cell.forEachChild([&todo, &root, &cell, &func](const Cell& child) {
        if(child.isA<const Collectible>() &&
           todo.find(child) == todo.end() &&
           root.find(child) == root.end()) {
          todo.insert(child);
        }
      });
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// test and debug
//
////////////////////////////////////////////////////////////////////////////////
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
    bool ret = true;
    auto fromColor = getFromColor();
    for(const Cell & cell : cells)
    {
      cell.forEachChild([&ret, fromColor](const Cell & child){
          if(child.isA<const Collectible>())
          {
            if(child.as<const Collectible>()->getColor() == fromColor)
            {
              ret = false;
            }
          }
        });
    }
    return ret;
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



