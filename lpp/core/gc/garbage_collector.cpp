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
    consMap(this),
    containers({CollectibleContainer<Container>(Lisp::Color::White, false, this),
          CollectibleContainer<Container>(Lisp::Color::Grey, false, this),
          CollectibleContainer<Container>(Lisp::Color::Black, false, this),
          CollectibleContainer<Container>(Lisp::Color::White,true, this),
          CollectibleContainer<Container>(Lisp::Color::Grey,true, this),
          CollectibleContainer<Container>(Lisp::Color::Black,true, this) }),
    garbageSteps(_garbageSteps),
    recycleSteps(_recycleSteps),
    backGarbageSteps(_garbageSteps),
    backRecycleSteps(_recycleSteps)
{
}

void GarbageCollector::forEachContainer(const CollectibleContainer<Container> & containers,
                                        std::function<void(const Cell &)> func) const
{
  for(auto itr = containers.cbegin(); itr != containers.cend(); ++itr)
  {
    func(Cell(*itr, (*itr)->getTypeId()));
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// forEachCollectible
//
////////////////////////////////////////////////////////////////////////////////
void GarbageCollector::forEachReachable(std::function<void(const Cell &)> func) const
{
  std::unordered_set<Cell> todo;
  std::unordered_set<Cell> root;
  forEachRootCollectible([&todo](const Cell & cell){
      todo.insert(cell);
    });
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
  std::vector<Cons*> conses;
  std::vector<Container*> containers;
  forEachReachable([&conses, &containers]
                   (const Cell & cell){
      if(cell.isA<Cons>())
      {
        auto c = cell.as<Cons>();
        if(!c->isRoot() && c->getColor() != Color::Black)
        {
          conses.push_back(c);
        }
      }
      else if(cell.isA<Container>())
      {
        auto c = cell.as<Container>();
        if(!c->isRoot() && c->getColor() != Color::Black)
        {
          containers.push_back(c);
        }
      }
  });
  consMap.swap(conses);
  Cons * cons;
  while((cons = consMap.popDisposed()))
  {
    cons->unsetNonCollectibleChildren();
    consPages.recycle(cons);
  }

  /*
  //@todo: make it generic for all collectible
  std::size_t nRoot = 0;
  std::unordered_set<Cons*> root;
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
  */
}

void GarbageCollector::stepCollector()
{
  for(unsigned short i=0; i < garbageSteps; i++)
  {
    bool swapable = true;
    swapable &= consMap.step();
    /*@todo other containers */
    if(swapable)
    {
      consMap.swap();
    }
  }
}

void Lisp::GarbageCollector::stepRecycle()
{
  /* @todo recycle contaienr, create tmp. object in garbageCollector
     if object is null -> popBack
     if not null unsetNonCollectibleChildren
     if return true -> delete obj */
  std::size_t i = recycleSteps;
  Cons * cons;
  while(i && (cons = consMap.popDisposed()))
  {
    cons->unsetNonCollectibleChildren();
    consPages.recycle(cons);
    i--;
  }
}


//////////////////////////////////////////////////
// @todo: check if the following is still needed
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


