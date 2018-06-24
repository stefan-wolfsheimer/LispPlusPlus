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
#include <lpp/core/gc/color_map.h>
#include <lpp/core/gc/cons_pages.h>
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/gc/unmanaged_collectible_container.h>
#include <lpp/core/gc/collectible_graph.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/container.h>

#define CONS_PAGE_SIZE 512

// @todo move to config.h
#define CONS_PAGE_SIZE 512

namespace Lisp
{
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

    template<typename C>
    inline C * make();

    inline std::size_t numCollectible() const;
    inline std::size_t numRootCollectible() const;
    inline std::size_t numBulkCollectible() const;
    inline std::size_t numCollectible(Color color) const;
    inline std::size_t numRootCollectible(Color color) const;
    inline std::size_t numBulkCollectible(Color color) const;
    inline std::size_t numVoidCollectible() const;
    inline std::size_t numDisposedCollectible() const;

    inline std::vector<Cell> get(void(GarbageCollector::*func)(std::function<void(const Cell &)> func) const) const;
    inline std::vector<Cell> get(Color color,
                                 void(GarbageCollector::*func)(Color color,
                                                               std::function<void(const Cell &)> func) const) const;

    inline void forEachCollectible(std::function<void(const Cell &)> func) const;
    inline void forEachRootCollectible(std::function<void(const Cell &)> func) const;
    inline void forEachBulkCollectible(std::function<void(const Cell &)> func) const;
    inline void forEachCollectible(Color color, std::function<void(const Cell &)> func) const;
    inline void forEachRootCollectible(Color color, std::function<void(const Cell &)> func) const;
    inline void forEachBulkCollectible(Color color, std::function<void(const Cell &)> func) const;
    void forEachDisposedCollectible(std::function<void(const Cell &)> func) const;

    void forEachReachable(std::function<void(const Cell &)> func) const;

    inline void disableCollector();
    inline void disableRecycling();
    inline void enableCollector();
    inline void enableRecycling();

    inline std::size_t getConsCycles() const;
    inline std::size_t getContainerCycles() const;

    void cycleCollector();
    void stepCollector();
    void stepRecycle();

    // test and debug
    inline bool checkSanity() const;
    inline bool checkRootSanity() const;
    inline bool checkBulkSanity() const;
    inline bool checkSanity(Color color) const;
    inline bool checkRootSanity(Color color) const;
    inline bool checkBulkSanity(Color color) const;

  private:
    ColorMap<Cons> consMap;
    ColorMap<Container> containerMap;
    
    ConsPages consPages;
    unsigned short int garbageSteps;
    unsigned short int recycleSteps;
    unsigned short int backGarbageSteps;
    unsigned short int backRecycleSteps;

    void forEachCons(const CollectibleContainer<Cons> & conses,
                     std::function<void(const Cell &)> func) const;
    void forEachContainer(const CollectibleContainer<Container> & containers,
                          std::function<void(const Cell &)> func) const;
    inline Cons * makeCons();
    inline void initContainer(Container *);
    inline bool checkSanity(Color color, bool root) const;
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
  ret->setRefCount(0u);
  consMap.add(ret);
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

inline void Lisp::GarbageCollector::initContainer(Container * container)
{
  container->setRefCount(0u);
  containerMap.add(container);
}

template<typename C>
inline C * Lisp::GarbageCollector::make()
{
  stepCollector();
  stepRecycle();
  C * ret = new C;
  initContainer(ret);
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// numCollectible
//
////////////////////////////////////////////////////////////////////////////////
inline std::size_t Lisp::GarbageCollector::numCollectible() const
{
  return numRootCollectible()  + numBulkCollectible();
}

inline std::size_t Lisp::GarbageCollector::numRootCollectible() const
{
  return
    numRootCollectible(Color::White) +
    numRootCollectible(Color::Grey) +
    numRootCollectible(Color::Black);
}

inline std::size_t Lisp::GarbageCollector::numBulkCollectible() const
{
  return
    numBulkCollectible(Color::White) +
    numBulkCollectible(Color::Grey) +
    numBulkCollectible(Color::Black);
}

inline std::size_t Lisp::GarbageCollector::numCollectible(Color color) const
{
  return numRootCollectible(color) + numBulkCollectible(color);
}

inline std::size_t Lisp::GarbageCollector::numRootCollectible(Color color) const
{
  return consMap.rootSize(color) + containerMap.rootSize(color);
}

inline std::size_t Lisp::GarbageCollector::numBulkCollectible(Color color) const
{
  return consMap.size(color) + containerMap.size(color);
}

inline std::size_t Lisp::GarbageCollector::numVoidCollectible() const
{
  return consPages.getNumVoid();
}

inline std::size_t Lisp::GarbageCollector::numDisposedCollectible() const
{
  return consMap.numDisposed() + containerMap.numDisposed();
}

////////////////////////////////////////////////////////////////////////////////
//
// forEachCollectible
//
////////////////////////////////////////////////////////////////////////////////
inline std::vector<Lisp::Cell>
Lisp::GarbageCollector::get(void(GarbageCollector::*func)(std::function<void(const Cell &)> func) const) const
{
  std::vector<Lisp::Cell> ret;
  (this->*func)([&ret](const Cell & cell) { ret.push_back(cell); });
  return ret;
}

inline std::vector<Lisp::Cell>
Lisp::GarbageCollector::get(Color color,
                            void(GarbageCollector::*func)(Color color,
                                                          std::function<void(const Cell &)> func) const) const
{
  std::vector<Lisp::Cell> ret;
  (this->*func)(color, [&ret](const Cell & cell) { ret.push_back(cell); });
  return ret;
}
 
inline void Lisp::GarbageCollector::forEachCollectible(std::function<void(const Cell &)> func) const
{
  forEachRootCollectible(func);
  forEachBulkCollectible(func);
}


inline void Lisp::GarbageCollector::forEachRootCollectible(std::function<void(const Cell &)> func) const
{
  forEachRootCollectible(Color::White, func);
  forEachRootCollectible(Color::Grey, func);
  forEachRootCollectible(Color::Black, func);
}

inline void Lisp::GarbageCollector::forEachBulkCollectible(std::function<void(const Cell &)> func) const
{
  forEachBulkCollectible(Color::White, func);
  forEachBulkCollectible(Color::Grey, func);
  forEachBulkCollectible(Color::Black, func);
}

inline void Lisp::GarbageCollector::forEachCollectible(Color color, std::function<void(const Cell &)> func) const
{
  forEachBulkCollectible(color, func);
  forEachRootCollectible(color, func);
}

inline void Lisp::GarbageCollector::forEachRootCollectible(Color color, std::function<void(const Cell &)> func) const
{
  consMap.forEachRoot(color, func);
  containerMap.forEachRoot(color, func);
}

inline void Lisp::GarbageCollector::forEachBulkCollectible(Color color, std::function<void(const Cell &)> func) const
{
  consMap.forEachBulk(color, func);
  containerMap.forEachBulk(color, func);
}

inline void Lisp::GarbageCollector::forEachDisposedCollectible(std::function<void(const Cell &)> func) const
{
  // todo: implement
}

////////////////////////////////////////////////////////////////////////////////
//
// enable / disable collector
//
////////////////////////////////////////////////////////////////////////////////
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

inline std::size_t Lisp::GarbageCollector::getConsCycles() const
{
  return consMap.getCycles();
}

inline std::size_t Lisp::GarbageCollector::getContainerCycles() const
{
  // @todo implement
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// test and debug
//
////////////////////////////////////////////////////////////////////////////////
inline bool Lisp::GarbageCollector::checkSanity() const
{
  return checkRootSanity() && checkBulkSanity();
}

inline bool Lisp::GarbageCollector::checkRootSanity() const
{
  return
    checkRootSanity(Color::White) &&
    checkRootSanity(Color::Grey) &&
    checkRootSanity(Color::Black);
}

inline bool Lisp::GarbageCollector::checkBulkSanity() const
{
  return
    checkBulkSanity(Color::White) &&
    checkBulkSanity(Color::Grey) &&
    checkBulkSanity(Color::Black);
}

inline bool Lisp::GarbageCollector::checkSanity(Color color) const
{
  return checkRootSanity(color) && checkBulkSanity(color);
}

inline bool Lisp::GarbageCollector::checkRootSanity(Color color) const
{
  return checkSanity(color, true);
}

inline bool Lisp::GarbageCollector::checkBulkSanity(Color color) const
{
  return checkSanity(color, false);
}

inline bool Lisp::GarbageCollector::checkSanity(Color color, bool root) const
{
  typedef void(GarbageCollector::*Function)(Color color,
                                            std::function<void(const Cell &)> func) const;
  Function f;
  if(root)
  {
    f = &GarbageCollector::forEachRootCollectible;
  }
  else
  {
    f = &GarbageCollector::forEachBulkCollectible;
  }
  (this->*f)(color, [this, color, root](const Cell & cell) {
      if(!cell.isA<Collectible>())
      {
        return false;
      }
      if(cell.getColor() != color)
      {
        return false;
      }
      if(cell.isRoot() != root)
      {
        return false;
      }
      if(!cell.checkIndex())
      {
        return false;
      }
      if(color == Color::Black)
      {
        bool ret = true;
        cell.forEachChild([&ret](const Cell & child){
            if(child.getColor() == Color::White)
            {
              ret = false;
            }
          });
        if(!ret)
        {
          return false;
        }
      }
    });
  return true;
}


