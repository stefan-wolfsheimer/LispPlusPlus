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
#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <core/gc/collectible_container.h>
#include <core/gc/unmanaged_collectible_container.h>
#include <core/gc/color.h>
#include <core/gc/garbage_collector.h>

// @todo move to config.h
#define CONS_PAGE_SIZE 512

namespace Lisp
{
  class Object;
  class Cell;
  class Cons;
  //todo: remove
  class ConsContainer;
  class Array;
  // todo: eventually replace by GC
  class ConsFactory : public GarbageCollector
  {
  public:
    using Color = Lisp::Color;
    ConsFactory(std::size_t _pageSize=CONS_PAGE_SIZE,
                unsigned short _garbageSteps=1,
                unsigned short _recycleSteps=1);
    ~ConsFactory();

    void disableGarbageCollector();
    void disableGarbageRecycling();
    void enableGarbageCollector();
    void enableGarbageRecycling();

    Color getFromColor() const;
    Color getToColor() const;
    Color getFromRootColor() const;
    Color getToRootColor() const;

    /**
     * Allocate and initialize a new Cons object in the root set.
     * Reference count is 0.
     */
    Cons * make(const Object & car, const Object & cdr);
    Cons * make(Object && car, const Object & cdr);
    Cons * make(const Object & car, Object && cdr);
    Cons * make(Object && car, Object && cdr);

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

    /**
     * Move cons to root set and set the color to getFromRootColor()
     * A step if the garbage collector is executed.
     */
    void root(Cons * cons);
    void unroot(Cons * cons);

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

    /**
     * The number of conses with a given color
     * @return Number of conses
     */
    std::size_t numConses(Color color) const;

    /**
     * The number of root conses
     * @return Number of conses
     */
    std::size_t numRootConses() const;

    /**
     * Compute the number of reachable conses.
     * @return Number of reachable conses.
     */
    std::size_t numReachableConses() const;

    std::vector<Cell> getCollectible() const;
    std::vector<Cell> getCollectible(Color color) const;
    std::vector<Cell> getRootCollectible() const;
    std::vector<Cell> getReachable() const;

    // todo replace with getCollectible
    std::vector<Cons*> getConses(Color color) const;
    // todo replace with getRootCollectible
    std::vector<Cons*> getRootConses() const;
    // todo replace with reachable 
    std::vector<Cons*> getReachableConses() const;
    std::unordered_set<const Cons*> getReachableConsesAsConstSet() const;
    std::unordered_set<Cons*> getReachableConsesAsSet() const;

    void cycleGarbageCollector();
    bool stepGarbageCollector(ConsContainer * container);
    void stepGarbageCollector();
    void stepRecycle();

  private:
    inline Cons * make();
    inline void greyChildInternal(Cons * cons);
    inline void greyChildInternal(const Cell & cell);

    template<typename T>
    inline std::unordered_set<T*> getReachableConsesAsSetIntneral() const;
    unsigned short int garbageSteps;
    unsigned short int recycleSteps;
    unsigned short int backGarbageSteps;
    unsigned short int backRecycleSteps;
    std::size_t pageSize;
    Color fromColor;
    Color toColor;
    Color fromRootColor;
    Color toRootColor;
    std::vector<Cons*> pages;
    CollectibleContainer<Cons> conses[7];
    UnmanagedCollectibleContainer<Cons> freeConses;

    std::vector<ConsContainer*> consContainers[7];

    std::vector<Array*> arrays[7];
  };
}

