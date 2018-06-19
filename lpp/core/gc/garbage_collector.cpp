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
#include <lpp/core/types/container.h>

using GarbageCollector = Lisp::GarbageCollector;
using CollectibleGraph = Lisp::CollectibleGraph;

using Cell = Lisp::Cell;

GarbageCollector::GarbageCollector(std::size_t consPageSize,
                                   unsigned short _garbageSteps,
                                   unsigned short _recycleSteps)
  : consPages(consPageSize),
    conses({CollectibleContainer<Cons>(Lisp::Color::White, this),
            CollectibleContainer<Cons>(Lisp::Color::Grey, this),
            CollectibleContainer<Cons>(Lisp::Color::Black, this),
            CollectibleContainer<Cons>(Lisp::Color::WhiteRoot, this),
            CollectibleContainer<Cons>(Lisp::Color::GreyRoot, this),
            CollectibleContainer<Cons>(Lisp::Color::BlackRoot, this) }),
    containers({CollectibleContainer<Container>(Lisp::Color::White, this),
                CollectibleContainer<Container>(Lisp::Color::Grey, this),
                CollectibleContainer<Container>(Lisp::Color::Black, this),
                CollectibleContainer<Container>(Lisp::Color::WhiteRoot, this),
                CollectibleContainer<Container>(Lisp::Color::GreyRoot, this),
                CollectibleContainer<Container>(Lisp::Color::BlackRoot, this) }),
    garbageSteps(_garbageSteps),
    recycleSteps(_recycleSteps),
    backGarbageSteps(_garbageSteps),
    backRecycleSteps(_recycleSteps)
{
  setToColor(Color::Black);
  conses[(unsigned short)Color::BlackRoot].otherElements = &conses[(unsigned short)Color::Black];
  conses[(unsigned short)Color::GreyRoot].otherElements = &conses[(unsigned short)Color::Grey];
  conses[(unsigned short)Color::WhiteRoot].otherElements = &conses[(unsigned short)Color::White];
  conses[(unsigned short)Color::Grey].greyElements = nullptr;
  conses[(unsigned short)Color::GreyRoot].greyElements = nullptr;
}

void GarbageCollector::forEachCons(const CollectibleContainer<Cons> & conses,
                                   std::function<void(const Cell &)> func) const
{
  for(auto itr = conses.cbegin(); itr != conses.cend(); ++itr)
  {
    func(*itr);
  }
}

void GarbageCollector::forEachContainer(const CollectibleContainer<Container> & containers,
                                        std::function<void(const Cell &)> func) const
{
  for(auto itr = containers.cbegin(); itr != containers.cend(); ++itr)
  {
    func(Cell(*itr, (*itr)->getTypeId()));
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
  forEachContainer(containers[(unsigned char)color], func);
}

void GarbageCollector::forEachDisposedCollectible(std::function<void(const Cell &)> func) const
{
  // todo: implement
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
// process garbage collector
//
////////////////////////////////////////////////////////////////////////////////
void GarbageCollector::cycleCollector()
{
  //@todo: make it generic for all collectible
  std::size_t nRoot = 0;
  std::unordered_set<Cons*> root;
  forEachReachable([&root](const Cell & cell){
      if(cell.isA<Cons>())
      {
        root.insert(cell.as<Cons>());
      }
  });
  for(auto cons : root)
  {
    if(cons->isRoot())
    {
      nRoot++;
    }
  }
  assert(root.size() >= nRoot);
  //assert(pages.size() * pageSize > root.size());
  setToColor(Color::Black);
  conses[(unsigned char)toColor].elements.clear();
  conses[(unsigned char)toColor].elements.reserve(root.size() - nRoot);
  conses[(unsigned char)Color::Grey].elements.clear();
  conses[(unsigned char)fromRootColor].elements.reserve(conses[(unsigned char)fromRootColor].size() + conses[(unsigned char)toRootColor].size());
  for(auto obj : conses[(unsigned char)toRootColor].elements)
  {
    assert(root.find(obj) != root.end());
    conses[(unsigned char)fromRootColor].add(obj);
  }
  conses[(unsigned char)toRootColor].elements.clear();
  conses[(unsigned char)fromColor].elements.clear();
  consPages.recycleAll(root, conses[(unsigned char)toColor], fromRootColor);
}

/*
bool GarbageCollector::stepCollector(ConsContainer* container)
{
  if(container->gcTop < container->conses.size())
  {
    auto cons = container->conses[container->gcTop];
    if(cons->getColor() == fromRootColor)
    {
      conses[(unsigned char)cons->getColor()].remove(cons);
      conses[(unsigned char)toRootColor].add(cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else
    {
      assert(cons->getColor() == toRootColor);
    }
    container->gcTop++;
    return false;
  }
  else
  {
    return true;
  }
  } */

void GarbageCollector::stepCollector()
{
  //Todo lock
  for(unsigned short i=0; i < garbageSteps; i++)
  {
    /*if(!consContainers[(unsigned char)(fromRootColor)].empty())
    {
      auto container = consContainers[(unsigned char)fromRootColor].back();
      assert(container->color == fromRootColor);
      assert(container->index == consContainers[(unsigned char)fromRootColor].size()-1);
      if(stepCollector(container))
      {
        consContainers[(unsigned char)fromRootColor].pop_back();
        container->index = consContainers[(unsigned char)toRootColor].size();
        container->color = toRootColor;
        consContainers[(unsigned char)toRootColor].push_back(container);
        container->gcTop = 0;
      }
    }
    else*/ if(!conses[(unsigned char)(fromRootColor)].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)fromRootColor].popBack();
      assert(cons->getColor() == fromRootColor);
      assert(cons->getIndex() == conses[(unsigned char)fromRootColor].size());
      cons->getCarCell().grey();
      cons->getCdrCell().grey();
      conses[(unsigned char)toRootColor].add(cons);
    }
    else if(!conses[(unsigned char)Color::GreyRoot].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)Color::GreyRoot].popBack();
      assert(cons->getColor() == Color::GreyRoot);
      assert(cons->getIndex() == conses[(unsigned char)Color::GreyRoot].size());
      cons->getCarCell().grey();
      cons->getCdrCell().grey();
      conses[(unsigned char)toRootColor].add(cons);
    }
    else if(!conses[(unsigned char)Color::Grey].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)Color::Grey].popBack();
      assert(cons->getColor() == Color::Grey);
      assert(cons->getIndex() == conses[(unsigned char)Color::Grey].size());
      cons->getCarCell().grey();
      cons->getCdrCell().grey();
      conses[(unsigned char)toColor].add(cons);
    }
    else
    {
      // Todo test swap
      assert(conses[(unsigned char)Color::Grey].empty());
      assert(conses[(unsigned char)Color::GreyRoot].empty());
      assert(conses[(unsigned char)fromRootColor].empty());
      disposedConses.move(conses[(unsigned char)fromColor]);
      setToColor(toColor == Color::White ? Color::Black : Color::White);
    }
  }
}

void Lisp::GarbageCollector::stepRecycle()
{
  std::size_t i = recycleSteps;
  Cons * cons;
  while(i && (cons = disposedConses.popBack()))
  {
    if(!cons->getCarCell().isA<Cons>())
    {
      cons->unsetCar();
    }
    if(!cons->getCdrCell().isA<Cons>())
    {
      cons->unsetCdr();
    }
    //todo move to another unmanged container (because we don't need
    // index and color managedment overhead
    consPages.recycle(cons);
    i--;
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

//////////////////////////////////////////////////
// todo: check if the following is still needed
//////////////////////////////////////////////////
/*Lisp::Array * GarbageCollector::makeArray()
{
  Color color = fromRootColor;
  auto ret = new Array(this, color, arrays[(unsigned char)color].size());
  arrays[(unsigned char)color].push_back(ret);
  return ret;
  } */

/*Lisp::ConsContainer * GarbageCollector::makeContainer()
{
  Color color = fromRootColor;
  auto ret = new ConsContainer(this,
                               color,
                               consContainers[(unsigned char)color].size());
  consContainers[(unsigned char)color].push_back(ret);
  return ret;
  }*/


