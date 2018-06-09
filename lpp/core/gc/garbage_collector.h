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

namespace Lisp
{
  class GarbageCollector
  {
  public:
    GarbageCollector(std::size_t consPageSize,
                     unsigned short _garbageSteps=1,
                     unsigned short _recycleSteps=1);

    inline Color getFromColor() const;
    inline Color getToColor() const;
    inline Color getFromRootColor() const;
    inline Color getToRootColor() const;

    inline std::vector<Cell> getCollectible() const;
    inline std::vector<Cell> getCollectible(Color color) const;
    inline std::vector<Cell> getDisposedCollectible() const;
    inline std::vector<Cell> getRootCollectible() const;
    inline std::vector<Cell> getReachable() const;
    void forEachCollectible(std::function<void(const Cell &)> func) const;
    void forEachCollectible(Color color, std::function<void(const Cell &)> func) const;
    void forEachDisposedCollectible(std::function<void(const Cell &)> func) const;
    void forEachRootCollectible(std::function<void(const Cell &)> func) const;
    void forEachReachable(std::function<void(const Cell &)> func) const;

    // test and debug
    bool checkSanity(Color color) const;
    bool checkSanity() const;
    
  protected: //todo make private
    CollectibleContainer<Cons> conses[7]; // todo: reduce number of colors
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
                     
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
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

