/******************************************************************************
Copyright (c) 2017, Stefan Wolfsheimer

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
#include <assert.h>
#include <unordered_set>
#include "lisp_cons_factory.h"
#include "types/cons.h"
#include "types/lisp_array.h"

// todo: remove cons_container
#include "types/lisp_cons_container.h"

using Cell = Lisp::Cell;
using Cons = Lisp::Cons;
using ConsFactory = Lisp::ConsFactory;
using ConsContainer = Lisp::ConsContainer;
using Array = Lisp::Array;

Lisp::ConsFactory::ConsFactory(std::size_t _pageSize,
                               unsigned short _garbageSteps,
                               unsigned short _recycleSteps) :
   GarbageCollector(_pageSize, _garbageSteps, _recycleSteps)
{
}

Lisp::ConsFactory::~ConsFactory()
{
}

void ConsFactory::disableGarbageCollector()
{
  backGarbageSteps = garbageSteps;
  garbageSteps = 0;
}

void ConsFactory::disableGarbageRecycling()
{
  backRecycleSteps = recycleSteps;
  recycleSteps = 0;
}

void ConsFactory::enableGarbageCollector()
{
  garbageSteps = backGarbageSteps;
}

void ConsFactory::enableGarbageRecycling()
{
  recycleSteps = backRecycleSteps;
}

void Lisp::ConsFactory::gcStep(Cons * cons)
{
  Color color = cons->getColor();
  if(color == fromColor || color == Color::Grey)
  {
    conses[(unsigned char)cons->color].remove(cons);
    conses[(unsigned char)toColor].add(cons);
  }
  else if(color == fromRootColor || color == Color::GreyRoot)
  {
    conses[(unsigned char)cons->color].remove(cons);
    conses[(unsigned char)toRootColor].add(cons);
  }
  greyChildInternal(cons->getCarCell());
  greyChildInternal(cons->getCdrCell());
}

inline void Lisp::ConsFactory::greyChildInternal(const Cell & cell)
{
  auto cons = cell.as<Cons>();
  if(cons)
  {
    greyChildInternal(cons);
  }
}

inline void Lisp::ConsFactory::greyChildInternal(Cons * cons)
{
  if(cons->getColor() == fromColor)
  {
    conses[(unsigned char)cons->color].remove(cons);
    conses[(unsigned char)Color::Grey].add(cons);
  }
  else if(cons->getColor() == fromRootColor)
  {
    conses[(unsigned char)cons->color].remove(cons);
    conses[(unsigned char)Color::GreyRoot].add(cons);
  }
}

inline Cons * ConsFactory::make()
{
  stepGarbageCollector();
  stepRecycle();
  Cons * ret = consPages.next();
  ret->refCount = 0;
  ret->consFactory = this;
  // new cons is root with from-color
  conses[(unsigned char)(toColor == Cons::Color::Black ?
                         Cons::Color::WhiteRoot:
                         Cons::Color::BlackRoot)].add(ret);
  return ret;
}

Cons * ConsFactory::make(const Object & car, const Object & cdr)
{
  Cons * ret = make();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

Cons * ConsFactory::make(Object && car, const Object & cdr)
{
  Cons * ret = make();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

Cons * ConsFactory::make(const Object & car, Object && cdr)
{
  Cons * ret = make();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

Cons * ConsFactory::make(Object && car, Object && cdr)
{
  Cons * ret = make();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

Array * ConsFactory::makeArray()
{
  Color color = (toColor == Color::Black ? Color::WhiteRoot : Color::BlackRoot);
  auto ret = new Array(this, color, arrays[(unsigned char)color].size());
  arrays[(unsigned char)color].push_back(ret);
  return ret;
}

ConsContainer * ConsFactory::makeContainer()
{
  Color color = toColor == Color::Black ? Color::WhiteRoot : Color::BlackRoot;
  auto ret = new ConsContainer(this,
                               color,
                               consContainers[(unsigned char)color].size());
  consContainers[(unsigned char)color].push_back(ret);
  return ret;
}

void Lisp::ConsFactory::root(Cons * cons)
{
  assert(!cons->isRoot());
  assert(cons->color != Cons::Color::Void);
  assert(cons->index < conses[(unsigned char)cons->color].size());
  //todo: make assertion work
  //assert(cons == &*conses[(unsigned char)cons->color][cons->index]);
  conses[(unsigned char)cons->color].remove(cons);
  cons->refCount = 1u;
  // new root with from-color
  conses[(unsigned char)(toColor == Cons::Color::Black ?
                         Cons::Color::WhiteRoot :
                         Cons::Color::BlackRoot)].add(cons);
  stepGarbageCollector();
  stepRecycle();
}

void Lisp::ConsFactory::unroot(Cons * cons)
{
  assert(cons->isRoot());
  // todo make assertion work again
  //  assert(cons == &*conses[(unsigned char)cons->color][cons->index]);
  conses[(unsigned char)cons->color].remove(cons);
  conses[(unsigned char)cons->color - 3].add(cons);
  stepGarbageCollector();
  stepRecycle();
}

std::size_t Lisp::ConsFactory::numConses(Color color) const
{
  if(color == Color::Free)
  {
    return freeConses.size();
  }
  else if(color == Color::Void)
  {
    return consPages.getNumVoid();
  }
  else
  {
    return conses[(unsigned char)color].size();
  }
}

std::size_t Lisp::ConsFactory::numRootConses() const
{
  return
    conses[(unsigned char)Color::WhiteRoot].size() +
    conses[(unsigned char)Color::GreyRoot].size() +
    conses[(unsigned char)Color::BlackRoot].size();
}

std::size_t Lisp::ConsFactory::numReachableConses() const
{
  return getReachableConsesAsConstSet().size();
}

std::vector<Cons*> Lisp::ConsFactory::getConses(Color color) const
{
  // todo: replace get cells
  std::vector<Cons*> ret;
  if(color == Color::Free)
  {
    //std::vector<Cell> tmp;
    //for(auto & c : tmp)
    //{
    //  ret.push_back(c.as<Cons>());
    //}
  }
  else
  {
    for(Cons * v: conses[(unsigned char)color].elements)
    {
      ret.push_back(v);
    }
  }
  return ret;
}

std::vector<Cons*> Lisp::ConsFactory::getRootConses() const
{
  std::vector<Cons*> ret;
  ret.insert(ret.end(),
             conses[(unsigned char)Color::BlackRoot].elements.begin(),
             conses[(unsigned char)Color::BlackRoot].elements.end());
  ret.insert(ret.end(),
             conses[(unsigned char)Color::GreyRoot].elements.begin(),
             conses[(unsigned char)Color::GreyRoot].elements.end());
  ret.insert(ret.end(),
             conses[(unsigned char)Color::WhiteRoot].elements.begin(),
             conses[(unsigned char)Color::WhiteRoot].elements.end());
  return ret;
}

template<typename T> std::unordered_set<T*>
Lisp::ConsFactory::getReachableConsesAsSetIntneral() const
{
  typedef T ConsType;
  std::unordered_set<ConsType*> todo;
  std::unordered_set<ConsType*> root;
  todo.insert(conses[(unsigned char)Color::BlackRoot].elements.begin(),
              conses[(unsigned char)Color::BlackRoot].elements.end());
  todo.insert(conses[(unsigned char)Color::GreyRoot].elements.begin(),
              conses[(unsigned char)Color::GreyRoot].elements.end());
  todo.insert(conses[(unsigned char)Color::WhiteRoot].elements.begin(),
              conses[(unsigned char)Color::WhiteRoot].elements.end());
  while(!todo.empty())
  {
    ConsType * cons = *todo.begin();
    todo.erase(cons);
    root.insert(cons);
    {
      const Cell & ocar(cons->car);
      ConsType * car = ocar.as<ConsType>();
      if(car && todo.find(car) == todo.end() && root.find(car) == root.end())
      {
        todo.insert(car);
      }
      const Cell & ocdr(cons->cdr);
      ConsType * cdr = ocdr.as<ConsType>();
      if(cdr && todo.find(cdr) == todo.end() && root.find(cdr) == root.end())
      {
        todo.insert(cdr);
      }
    }
  }
  return root;
}


std::unordered_set<const Cons*> Lisp::ConsFactory::getReachableConsesAsConstSet() const
{
  return getReachableConsesAsSetIntneral<const Cons>();
}

std::unordered_set<Cons*> Lisp::ConsFactory::getReachableConsesAsSet() const
{
  return getReachableConsesAsSetIntneral<Cons>();
}

std::vector<Cons*> Lisp::ConsFactory::getReachableConses() const
{
  //Todo lock
  std::unordered_set<Cons*> root = getReachableConsesAsSet();
  std::vector<Cons*> ret(root.begin(), root.end());
  return ret;
}

void Lisp::ConsFactory::cycleGarbageCollector()
{
  //Todo lock
  std::unordered_set<Cons*> root;
  std::size_t nRoot = 0;
  root = Lisp::ConsFactory::getReachableConsesAsSetIntneral<Cons>();
  for(auto cons : root)
  {
    if(cons->isRoot())
    {
      nRoot++;
    }
  }
  assert(root.size() >= nRoot);
  //assert(pages.size() * pageSize > root.size());
  fromColor = Color::White;
  toColor = Color::Black;
  fromRootColor = Color::WhiteRoot;
  toRootColor = Color::BlackRoot;
  conses[(unsigned char)toColor].elements.clear();
  conses[(unsigned char)toColor].elements.reserve(root.size() - nRoot);
  //conses[(unsigned char)Color::Void].elements.clear();
  //conses[(unsigned char)Color::Void].elements.reserve(pages.size() * pageSize - root.size());
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

bool Lisp::ConsFactory::stepGarbageCollector(ConsContainer* container)
{
  if(container->gcTop < container->conses.size())
  {
    auto cons = container->conses[container->gcTop];
    if(cons->color == fromRootColor)
    {
      conses[(unsigned char)cons->color].remove(cons);
      conses[(unsigned char)toRootColor].add(cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else
    {
      assert(cons->color == toRootColor);
    }
    container->gcTop++;
    return false;
  }
  else
  {
    return true;
  }
}

void Lisp::ConsFactory::stepGarbageCollector()
{
  //Todo lock
  for(unsigned short i=0; i < garbageSteps; i++)
  {
    if(!consContainers[(unsigned char)(fromRootColor)].empty())
    {
      auto container = consContainers[(unsigned char)fromRootColor].back();
      assert(container->color == fromRootColor);
      assert(container->index == consContainers[(unsigned char)fromRootColor].size()-1);
      if(stepGarbageCollector(container))
      {
        consContainers[(unsigned char)fromRootColor].pop_back();
        container->index = consContainers[(unsigned char)toRootColor].size();
        container->color = toRootColor;
        consContainers[(unsigned char)toRootColor].push_back(container);
        container->gcTop = 0;
      }
    }
    else if(!conses[(unsigned char)(fromRootColor)].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)fromRootColor].popBack();
      assert(cons->color == fromRootColor);
      assert(cons->index == conses[(unsigned char)fromRootColor].size());
      conses[(unsigned char)toRootColor].add(cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else if(!conses[(unsigned char)Color::GreyRoot].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)Color::GreyRoot].popBack();
      assert(cons->color == Color::GreyRoot);
      assert(cons->index == conses[(unsigned char)Color::GreyRoot].size());
      conses[(unsigned char)toRootColor].add(cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else if(!conses[(unsigned char)Color::Grey].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)Color::Grey].popBack();
      assert(cons->color == Color::Grey);
      assert(cons->index == conses[(unsigned char)Color::Grey].size());
      conses[(unsigned char)toColor].add(cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else
    {
      // Todo test swap
      assert(conses[(unsigned char)Color::Grey].empty());
      assert(conses[(unsigned char)Color::GreyRoot].empty());
      assert(conses[(unsigned char)fromRootColor].empty());
      freeConses.move(conses[(unsigned char)fromColor]);
      if(toColor == Color::White)
      {
        toColor = Color::Black;
        toRootColor = Color::BlackRoot;
        fromColor = Color::White;
        fromRootColor = Color::WhiteRoot;
      }
      else
      {
        toColor = Color::White;
        toRootColor = Color::WhiteRoot;
        fromColor = Color::Black;
        fromRootColor = Color::BlackRoot;
      }
    }
  }
}

void Lisp::ConsFactory::stepRecycle()
{
  std::size_t i = recycleSteps;
  Cons * cons;
  while(i && (cons = freeConses.popBack()))
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

