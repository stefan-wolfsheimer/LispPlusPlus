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


    void cycleCollector();
    void stepCollector();
    void stepRecycle();

    // test and debug
    bool checkSanity(Color color) const;
    bool checkSanity() const;

  private:
    CollectibleContainer<Cons> conses[6];
    UnmanagedCollectibleContainer<Cons> disposedConses;

    CollectibleContainer<Container> containers[6];
    UnmanagedCollectibleContainer<Container> disposedContainers;
    
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
    void forEachContainer(const CollectibleContainer<Container> & containers,
                          std::function<void(const Cell &)> func) const;
    inline Cons * makeCons();
    inline void setToColor(Color color);
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
  // new cons is root with from-color
  conses[(unsigned char)fromRootColor].add(ret);
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
  return conses[(unsigned char)color].size() + containers[(unsigned char)color].size();
}

inline std::size_t Lisp::GarbageCollector::numVoidCollectible() const
{
  return consPages.getNumVoid();
}

inline std::size_t Lisp::GarbageCollector::numDisposedCollectible() const
{
  return disposedConses.size() + disposedContainers.size();
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

inline void Lisp::GarbageCollector::setToColor(Color color)
{
  if(color == Color::Black)
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
  conses[(unsigned short)Color::Black].otherElements = &conses[(unsigned short)fromRootColor];
  conses[(unsigned short)Color::Grey].otherElements = &conses[(unsigned short)fromRootColor];
  conses[(unsigned short)Color::White].otherElements = &conses[(unsigned short)fromRootColor];
  conses[(unsigned short)fromColor].greyElements = &conses[(unsigned short)Color::Grey];
  conses[(unsigned short)fromRootColor].greyElements = &conses[(unsigned short)Color::GreyRoot];
  conses[(unsigned short)toColor].toElements = nullptr;
  conses[(unsigned short)toRootColor].toElements = nullptr;
  conses[(unsigned short)Color::Grey].toElements = &conses[(unsigned short)toColor];
  conses[(unsigned short)Color::GreyRoot].toElements = &conses[(unsigned short)toRootColor];
}

