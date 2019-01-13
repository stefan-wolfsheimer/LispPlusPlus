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
using Cell = Lisp::Cell;

GarbageCollector::~GarbageCollector()
{
  cycle();
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
void GarbageCollector::cycle()
{
  std::unordered_set<BasicCons*> conses;
  std::unordered_set<Container*> containers;
  forEachReachable([&conses, &containers]
                   (const Cell & cell){
      if(cell.isA<BasicCons>())
      {
        auto c = cell.as<BasicCons>();
        if(!c->isRoot())
        {
          conses.insert(c);
        }
      }
      else if(cell.isA<Container>())
      {
        auto c = cell.as<Container>();
        if(!c->isRoot())
        {
          containers.insert(c);
        }
      }
  });
  consMap.swap(conses);
  containerMap.swap(containers);
  cycles++;
  BasicCons * cons;
  while((cons = consMap.popDisposed()))
  {
    cons->recycleNextChild();
    consPages.recycle(cons);
  }
  if(toBeRecycled)
  {
    while(!toBeRecycled->recycleNextChild())
    {
      delete toBeRecycled;
    }
  }
  while((toBeRecycled = containerMap.popDisposed()))
  {
    toBeRecycled->resetGcPosition();
    while(!toBeRecycled->recycleNextChild())
    {}
    delete toBeRecycled;
  }
  toBeRecycled = nullptr;
}

void Lisp::GarbageCollector::recycle()
{
  std::size_t i = recycleSteps;
  BasicCons * cons;
  while(i && (cons = consMap.popDisposed()))
  {
    cons->recycleNextChild();
    consPages.recycle(cons);
    i--;
  }
  while(i)
  {
    if(!toBeRecycled)
    {
      toBeRecycled = containerMap.popDisposed();
      if(toBeRecycled)
      {
        toBeRecycled->resetGcPosition();
        if(toBeRecycled->recycleNextChild())
        {
          delete toBeRecycled;
          toBeRecycled = nullptr;
        }
      }
      else
      {
        break;
      }
    }
    else
    {
      if(toBeRecycled->recycleNextChild())
      {
        delete toBeRecycled;
        toBeRecycled = nullptr;
      }
    }
    --i;
  }
}
