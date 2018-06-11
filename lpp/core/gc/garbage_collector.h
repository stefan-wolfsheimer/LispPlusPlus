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
#pragma once
#include <vector>
#include <functional>
#include <lpp/core/gc/cons_pages.h>
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/gc/unmanaged_collectible_container.h>
#include <lpp/core/gc/collectible_graph.h>

#define CONS_PAGE_SIZE 512

// @todo move to config.h
#define CONS_PAGE_SIZE 512

namespace Lisp
{
  class ConsContainer;
  class Array;

  class GarbageCollector
  {
  public:
    GarbageCollector(std::size_t consPageSize=CONS_PAGE_SIZE,
                     unsigned short _garbageSteps=1,
                     unsigned short _recycleSteps=1);

    /**
     * Allocate and initialize a new Cons object in the root set.
     * Reference count is 0.
     */
    inline Cons * makeCons(const Object & car, const Object & cdr);
    inline Cons * makeCons(Object && car, const Object & cdr);
    inline Cons * makeCons(const Object & car, Object && cdr);
    inline Cons * makeCons(Object && car, Object && cdr);

    /**
     * Move cons to root set and set the color to getFromRootColor()
     * A step if the garbage collector is executed.
     */
    inline void root(Cons * cons);
    inline void unroot(Cons * cons);

    /**
     * Allocate and initialize a new Arry object in the root set.
     * Reference count is 0, color is FromRootColor
     */
    Array * makeArray();

    /**
     * Allocate and initialize a new ConsContainer object in the root set.
     * Reference count is 0.
     */
    ConsContainer * makeContainer();

    
    inline Color getFromColor() const;
    inline Color getToColor() const;
    inline Color getFromRootColor() const;
    inline Color getToRootColor() const;

    inline std::vector<Cell> getCollectible() const;
    inline std::vector<Cell> getCollectible(Color color) const;
    inline std::vector<Cell> getDisposedCollectible() const;
    inline std::vector<Cell> getRootCollectible() const;
    inline std::vector<Cell> getReachable() const;

    inline std::size_t numCollectible() const;
    inline std::size_t numCollectible(Color color) const;
    inline std::size_t numVoidCollectible() const;
    inline std::size_t numDisposedCollectible() const;
    inline std::size_t numRootCollectible() const;

    void forEachCollectible(std::function<void(const Cell &)> func) const;
    void forEachCollectible(Color color, std::function<void(const Cell &)> func) const;
    void forEachDisposedCollectible(std::function<void(const Cell &)> func) const;
    void forEachRootCollectible(std::function<void(const Cell &)> func) const;
    void forEachReachable(std::function<void(const Cell &)> func) const;

    inline void disableCollector();
    inline void disableRecycling();
    inline void enableCollector();
    inline void enableRecycling();

    /* collector processing 
     */
    /**
     * Performs a garbage collector step on cons
     *
     * If color is getFromColor() then change color to getToColor()
     * If color is getFromRootColor() then change color to getToRootColor()
     * If color is Color::Grey then  change color to getToColor()
     * If color is Color::GreyRoot then change color to getToRootColor()
     * All children having getFromColor() or getFromRootColor()) are changed
     * to Color::GreyRoot or Color::Grey.
     */
    void gcStep(Cons * cons);

    void cycleCollector();
    bool stepCollector(ConsContainer * container);
    void stepCollector();
    void stepRecycle();

    // test and debug
    bool checkSanity(Color color) const;
    bool checkSanity() const;

    

  protected: //todo make private
    CollectibleContainer<Cons> conses[7]; // todo: reduce number of colors
    std::vector<ConsContainer*> consContainers[7]; // todo: check if this is needed
    std::vector<Array*> arrays[7]; // todo: check if this is needed
    UnmanagedCollectibleContainer<Cons> freeConses; // todo: rename to disposed
    
    ConsPages consPages;
    unsigned short int garbageSteps;
    unsigned short int recycleSteps;
    unsigned short int backGarbageSteps;
    unsigned short int backRecycleSteps;
    Color fromColor;
    Color toColor;
    Color fromRootColor;
    Color toRootColor;
    void forEachCons(const CollectibleContainer<Cons> & conses,
                     std::function<void(const Cell &)> func) const;
  private:
    inline Cons * makeCons();
    inline void greyChildInternal(const Cell & cell);
    inline void greyChildInternal(Cons * cons);

  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
inline Lisp::Cons * Lisp::GarbageCollector::makeCons()
{
  stepCollector();
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

inline Lisp::Cons * Lisp::GarbageCollector::makeCons(const Object & car, const Object & cdr)
{
  Cons * ret = makeCons();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

inline Lisp::Cons * Lisp::GarbageCollector::makeCons(const Object & car, Object && cdr)
{
  Cons * ret = makeCons();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

inline Lisp::Cons * Lisp::GarbageCollector::makeCons(Object && car, const Object & cdr)
{
  Cons * ret = makeCons();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

inline Lisp::Cons * Lisp::GarbageCollector::makeCons(Object && car, Object && cdr)
{
  Cons * ret = makeCons();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// unroot
//
////////////////////////////////////////////////////////////////////////////////
void Lisp::GarbageCollector::root(Cons * cons)
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
  stepCollector();
  stepRecycle();
}

void Lisp::GarbageCollector::unroot(Cons * cons)
{
  assert(cons->isRoot());
  // todo make assertion work again
  //  assert(cons == &*conses[(unsigned char)cons->color][cons->index]);
  conses[(unsigned char)cons->color].remove(cons);
  conses[(unsigned char)cons->color - 3].add(cons);
  stepCollector();
  stepRecycle();
}

////////////////////////////////////////////////////////////////////////////////
//
// Color getter
//
////////////////////////////////////////////////////////////////////////////////
inline Lisp::Color Lisp::GarbageCollector::getFromColor() const
{
  return fromColor;
}

inline Lisp::Color Lisp::GarbageCollector::getToColor() const
{
  return toColor;
}

inline Lisp::Color Lisp::GarbageCollector::getFromRootColor() const
{
  return fromRootColor;
}

inline Lisp::Color Lisp::GarbageCollector::getToRootColor() const
{
  return toRootColor;
}

inline std::vector<Lisp::Cell> Lisp::GarbageCollector::getCollectible() const
{
  std::vector<Cell> ret;
  forEachCollectible([&ret](const Cell & c){
      ret.push_back(c);
  });
  return ret;
}

inline std::vector<Lisp::Cell> Lisp::GarbageCollector::getCollectible(Color color) const
{
  std::vector<Cell> ret;
  forEachCollectible(color, [&ret](const Cell & c){
      ret.push_back(c);
  });
  return ret;
}

inline std::vector<Lisp::Cell> Lisp::GarbageCollector::getDisposedCollectible() const
{
  std::vector<Cell> ret;
  forEachDisposedCollectible([&ret](const Cell & c){
      ret.push_back(c);
  });
  return ret;
}

inline std::vector<Lisp::Cell> Lisp::GarbageCollector::getRootCollectible() const
{
  std::vector<Cell> ret;
  forEachCollectible([&ret](const Cell & c){
      ret.push_back(c);
  });
  return ret;
}

inline std::vector<Lisp::Cell> Lisp::GarbageCollector::getReachable() const
{
  std::vector<Cell> ret;
  forEachReachable([&ret](const Cell & c){
      ret.push_back(c);
  });
  return ret;
}

inline std::size_t Lisp::GarbageCollector::numCollectible() const
{
  return
    numCollectible(Color::WhiteRoot) +
    numCollectible(Color::BlackRoot) +
    numCollectible(Color::GreyRoot) +
    numCollectible(Color::White) + 
    numCollectible(Color::Black) +
    numCollectible(Color::Grey);

}

inline std::size_t Lisp::GarbageCollector::numCollectible(Color color) const
{
  return conses[(unsigned char)color].size();
}

inline std::size_t Lisp::GarbageCollector::numVoidCollectible() const
{
  // todo: other
  return consPages.getNumVoid();
}

inline std::size_t Lisp::GarbageCollector::numDisposedCollectible() const
{
  // todo: other
  return 0;
}

inline std::size_t Lisp::GarbageCollector::numRootCollectible() const
{
  return
    numCollectible(Color::WhiteRoot) +
    numCollectible(Color::BlackRoot) +
    numCollectible(Color::GreyRoot);
}

inline void Lisp::GarbageCollector::disableCollector()
{
  backGarbageSteps = garbageSteps;
  garbageSteps = 0;
}

inline void Lisp::GarbageCollector::disableRecycling()
{
  backRecycleSteps = recycleSteps;
  recycleSteps = 0;
}

inline void Lisp::GarbageCollector::enableCollector()
{
  garbageSteps = backGarbageSteps;
}

inline void Lisp::GarbageCollector::enableRecycling()
{
  recycleSteps = backRecycleSteps;
}

inline void Lisp::GarbageCollector::gcStep(Cons * cons)
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
  
inline void Lisp::GarbageCollector::greyChildInternal(const Cell & cell)
{
  auto cons = cell.as<Cons>();
  if(cons)
  {
    greyChildInternal(cons);
  }
}

inline void Lisp::GarbageCollector::greyChildInternal(Cons * cons)
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


