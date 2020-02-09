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
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <assert.h>
#include <lpp/core/memory/color_map.h>
#include <lpp/core/memory/cons_pages.h>
#include <lpp/core/memory/collectible_container.h>
#include <lpp/core/memory/unmanaged_collectible_container.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/container.h>
#include <lpp/core/types/symbol.h>

// @todo move to config.h
#define CONS_PAGE_SIZE 512

namespace Lisp
{
  class Allocator
  {
  public:
    class Guard
    {
    public:
      Guard(Allocator & );
      Guard(Allocator * );
      ~Guard();
    private:
      Allocator & ref;
      unsigned short int garbageSteps;
      unsigned short int recycleSteps;
    };

    Allocator(std::size_t consPageSize=CONS_PAGE_SIZE,
                     unsigned short _garbageSteps=1,
                     unsigned short _recycleSteps=1);
    ~Allocator();


    template<typename C,  typename... ARGS>
    inline C * make(ARGS && ... rest);

    /**
     * Allocate and initialize a new Cons / Collectible object in the root set.
     * Reference count is 0.
     */
    template<typename C,  typename... ARGS>
    inline C * makeRoot(ARGS && ... rest);

    /**
     * Remove a symbol
     */
    inline void remove(Symbol * symbol);
    
    inline std::size_t numCollectible() const;
    inline std::size_t numRootCollectible() const;
    inline std::size_t numBulkCollectible() const;
    inline std::size_t numCollectible(Color color) const;
    inline std::size_t numRootCollectible(Color color) const;
    inline std::size_t numBulkCollectible(Color color) const;
    inline std::size_t numVoidCollectible() const;
    inline std::size_t numDisposedCollectible() const;

    inline std::vector<Cell> get(void(Allocator::*func)(std::function<void(const Cell &)> func) const) const;
    inline std::vector<Cell> get(Color color,
                                 void(Allocator::*func)(Color color,
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
    inline std::size_t getCycles() const;
    inline unsigned short getGarbageSteps() const;
    inline unsigned short getRecycleSteps() const;
    inline void setGarbageSteps(unsigned short steps);
    inline void setRecycleSteps(unsigned short steps);

    void cycle();
    inline void step();
    void recycle();

    // test and debug
    inline bool checkSanity() const;
    inline bool checkRootSanity() const;
    inline bool checkBulkSanity() const;
    inline bool checkSanity(Color color) const;
    inline bool checkRootSanity(Color color) const;
    inline bool checkBulkSanity(Color color) const;

  private:
    friend class Guard;
    ColorMap<BasicCons> consMap;
    ColorMap<Container> containerMap;
    std::unordered_map<std::string, Symbol*> symbols;
    Container * toBeRecycled;
    ConsPages consPages;
    unsigned short int garbageSteps;
    unsigned short int recycleSteps;
    unsigned short int backGarbageSteps;
    unsigned short int backRecycleSteps;
    std::size_t cycles;

    void forEachCons(const CollectibleContainer<Cons> & conses,
                     std::function<void(const Cell &)> func) const;
    void forEachContainer(const CollectibleContainer<Container> & containers,
                          std::function<void(const Cell &)> func) const;
    template<typename C>
    inline C * makeCons();

    template<typename C>
    inline C * makeRootCons();

    template<typename C>
    inline C * _make(ConsStorageTrait, const Cell & car, const Cell & cdr);

    template<typename C>
    inline C * _make(ConsStorageTrait, Cell && car, const Cell & cdr);

    template<typename C>
    inline C * _make(ConsStorageTrait, const Cell & car, Cell && cdr);

    template<typename C>
    inline C * _make(ConsStorageTrait, Cell && car, Cell && cdr);

    template<typename C>
    inline C * _makeRoot(ConsStorageTrait, const Cell & car, const Cell & cdr);

    template<typename C>
    inline C * _makeRoot(ConsStorageTrait, Cell && car, const Cell & cdr);

    template<typename C>
    inline C * _makeRoot(ConsStorageTrait, const Cell & car, Cell && cdr);

    template<typename C>
    inline C * _makeRoot(ConsStorageTrait, Cell && car, Cell && cdr);

    template<typename C,  typename... ARGS>
    inline C * _make(ContainerStorageTrait, ARGS&& ...rest);

    template<typename C,  typename... ARGS>
    inline C * _makeRoot(ContainerStorageTrait, ARGS&& ...rest);

    template<typename C,  typename... ARGS>
    inline C * _makeRoot(ManagedStorageTrait, ARGS&& ...rest);

    template<typename C>
    inline C * _make(SymbolStorageTrait, const std::string & name);

    template<typename C>
    inline C * _makeRoot(SymbolStorageTrait, const std::string & name);

    inline bool checkSanity(Color color, bool root) const;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////

inline Lisp::Allocator::Guard::Guard(Allocator & _ref)
  : ref(_ref)
{
  garbageSteps = ref.garbageSteps;
  recycleSteps = ref.recycleSteps;
  ref.garbageSteps = 0;
  ref.recycleSteps = 0;
}

inline Lisp::Allocator::Guard::Guard(Allocator * _ref)
  : ref(*_ref)
{
  garbageSteps = _ref->garbageSteps;
  recycleSteps = _ref->recycleSteps;
  _ref->garbageSteps = 0;
  _ref->recycleSteps = 0;
}


inline Lisp::Allocator::Guard::~Guard()
{
  ref.garbageSteps = garbageSteps;
  ref.recycleSteps = recycleSteps;
}

inline Lisp::Allocator::Allocator(std::size_t consPageSize,
                                                unsigned short _garbageSteps,
                                                unsigned short _recycleSteps)
  : consPages(consPageSize),
    consMap(this),
    containerMap(this),
    toBeRecycled(nullptr),
    garbageSteps(_garbageSteps),
    recycleSteps(_recycleSteps),
    backGarbageSteps(_garbageSteps),
    backRecycleSteps(_recycleSteps),
    cycles(0u)
{
}

template<typename C, typename... ARGS>
inline C * Lisp::Allocator::make(ARGS && ... rest)
{
  return _make<C>(typename TypeTraits<C>::StorageTrait(), std::forward<ARGS>(rest)...);
}

template<typename C,  typename... ARGS>
inline C * Lisp::Allocator::makeRoot(ARGS && ... rest)
{
  return _makeRoot<C>(typename TypeTraits<C>::StorageTrait(), std::forward<ARGS>(rest)...);
}

////////////////////////////////////////////////////////////////////////////////
//
// Cons
//
////////////////////////////////////////////////////////////////////////////////
template<typename C>
inline C * Lisp::Allocator::makeCons()
{
  // is derived from BasicCons and no members have been added
  typedef std::is_base_of<BasicCons, C> is_base_of_basic_cons;
  assert(is_base_of_basic_cons::value);
  assert(sizeof(C) == sizeof(BasicCons));

  step();
  recycle();
  BasicCons * ret = consPages.next();
  ret->refCount = 1u;
  consMap.add(ret);
  return static_cast<C*>(ret);
}

template<typename C>
inline C * Lisp::Allocator::makeRootCons()
{
  // is derived from BasicCons and no members have been added
  typedef std::is_base_of<BasicCons, C> is_base_of_basic_cons;
  assert(is_base_of_basic_cons::value);
  assert(sizeof(C) == sizeof(BasicCons));

  step();
  recycle();
  BasicCons * ret = consPages.next();
  ret->refCount = 1u;
  consMap.addRoot(ret);
  return static_cast<C*>(ret);
}

template<typename C>
inline C * Lisp::Allocator::_make(ConsStorageTrait, const Cell & car, const Cell & cdr)
{
  C * ret = makeCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_make(ConsStorageTrait, Cell && car, const Cell & cdr)
{
  C * ret = makeCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_make(ConsStorageTrait, const Cell & car, Cell && cdr)
{
  C * ret = makeCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_make(ConsStorageTrait, Cell && car, Cell && cdr)
{
  C * ret = makeCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_makeRoot(ConsStorageTrait, const Cell & car, const Cell & cdr)
{
  C * ret = makeRootCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_makeRoot(ConsStorageTrait, Cell && car, const Cell & cdr)
{
  C * ret = makeRootCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_makeRoot(ConsStorageTrait, const Cell & car, Cell && cdr)
{
  C * ret = makeRootCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

template<typename C>
inline C * Lisp::Allocator::_makeRoot(ConsStorageTrait, Cell && car, Cell && cdr)
{
  C * ret = makeRootCons<C>();
  ret->car = car;
  ret->cdr = cdr;
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
//
// Container
//
////////////////////////////////////////////////////////////////////////////////
template<typename C,  typename... ARGS>
inline C * Lisp::Allocator::_make(ContainerStorageTrait, ARGS&& ...rest)
{
  step();
  recycle();
  C * ret = new C(std::forward<ARGS>(rest)...);
  ret->refCount = 1u;
  containerMap.add(ret);
  ret->init();
  return ret;
}

template<typename C,  typename... ARGS>
inline C * Lisp::Allocator::_makeRoot(ContainerStorageTrait, ARGS&& ...rest)
{
  step();
  recycle();
  C * ret = new C(std::forward<ARGS>(rest)...);
  ret->refCount = 1u;
  containerMap.addRoot(ret);
  ret->init();
  return ret;
}

template<typename C,  typename... ARGS>
inline C * Lisp::Allocator::_makeRoot(ManagedStorageTrait, ARGS&& ...rest)
{
  return new C(std::forward<ARGS>(rest)...);
}

template<typename C>
inline C * Lisp::Allocator::_make(SymbolStorageTrait, const std::string & name)
{
  return _makeRoot<C>(SymbolStorageTrait(), name);
}

template<typename C>
inline C * Lisp::Allocator::_makeRoot(SymbolStorageTrait, const std::string & name)
{
  auto res = symbols.insert(std::make_pair(name, (Symbol*)nullptr));
  if(res.second)
  {
    res.first->second = new Symbol(res.first->first.c_str(), this);
    return res.first->second;
  }
  else
  {
    return res.first->second;
  }
}

inline void Lisp::Allocator::remove(Symbol * symbol)
{
  auto itr = symbols.find(symbol->getName());
  assert(itr != symbols.end());
  symbols.erase(itr);
}

////////////////////////////////////////////////////////////////////////////////
//
// numCollectible
//
////////////////////////////////////////////////////////////////////////////////
inline std::size_t Lisp::Allocator::numCollectible() const
{
  return numRootCollectible()  + numBulkCollectible();
}

inline std::size_t Lisp::Allocator::numRootCollectible() const
{
  return
    numRootCollectible(Color::White) +
    numRootCollectible(Color::Grey) +
    numRootCollectible(Color::Black);
}

inline std::size_t Lisp::Allocator::numBulkCollectible() const
{
  return
    numBulkCollectible(Color::White) +
    numBulkCollectible(Color::Grey) +
    numBulkCollectible(Color::Black);
}

inline std::size_t Lisp::Allocator::numCollectible(Color color) const
{
  return numRootCollectible(color) + numBulkCollectible(color);
}

inline std::size_t Lisp::Allocator::numRootCollectible(Color color) const
{
  return consMap.rootSize(color) + containerMap.rootSize(color);
}

inline std::size_t Lisp::Allocator::numBulkCollectible(Color color) const
{
  return consMap.size(color) + containerMap.size(color);
}

inline std::size_t Lisp::Allocator::numVoidCollectible() const
{
  return consPages.getNumVoid();
}

inline std::size_t Lisp::Allocator::numDisposedCollectible() const
{
  return consMap.numDisposed() + containerMap.numDisposed();
}

////////////////////////////////////////////////////////////////////////////////
//
// forEachCollectible
//
////////////////////////////////////////////////////////////////////////////////
inline std::vector<Lisp::Cell>
Lisp::Allocator::get(void(Allocator::*func)(std::function<void(const Cell &)> func) const) const
{
  std::vector<Lisp::Cell> ret;
  (this->*func)([&ret](const Cell & cell) { ret.push_back(cell); });
  return ret;
}

inline std::vector<Lisp::Cell>
Lisp::Allocator::get(Color color,
                            void(Allocator::*func)(Color color,
                                                          std::function<void(const Cell &)> func) const) const
{
  std::vector<Lisp::Cell> ret;
  (this->*func)(color, [&ret](const Cell & cell) { ret.push_back(cell); });
  return ret;
}
 
inline void Lisp::Allocator::forEachCollectible(std::function<void(const Cell &)> func) const
{
  forEachRootCollectible(func);
  forEachBulkCollectible(func);
}


inline void Lisp::Allocator::forEachRootCollectible(std::function<void(const Cell &)> func) const
{
  forEachRootCollectible(Color::White, func);
  forEachRootCollectible(Color::Grey, func);
  forEachRootCollectible(Color::Black, func);
}

inline void Lisp::Allocator::forEachBulkCollectible(std::function<void(const Cell &)> func) const
{
  forEachBulkCollectible(Color::White, func);
  forEachBulkCollectible(Color::Grey, func);
  forEachBulkCollectible(Color::Black, func);
}

inline void Lisp::Allocator::forEachCollectible(Color color, std::function<void(const Cell &)> func) const
{
  forEachBulkCollectible(color, func);
  forEachRootCollectible(color, func);
}

inline void Lisp::Allocator::forEachRootCollectible(Color color, std::function<void(const Cell &)> func) const
{
  consMap.forEachRoot(color, func);
  containerMap.forEachRoot(color, func);
}

inline void Lisp::Allocator::forEachBulkCollectible(Color color, std::function<void(const Cell &)> func) const
{
  consMap.forEachBulk(color, func);
  containerMap.forEachBulk(color, func);
}

inline void Lisp::Allocator::forEachDisposedCollectible(std::function<void(const Cell &)> func) const
{
  // todo: implement
}

////////////////////////////////////////////////////////////////////////////////
//
// enable / disable collector
//
////////////////////////////////////////////////////////////////////////////////
inline void Lisp::Allocator::disableCollector()
{
  if(garbageSteps != 0)
  {
    backGarbageSteps = garbageSteps;
    garbageSteps = 0;
  }
}

inline void Lisp::Allocator::disableRecycling()
{
  if(recycleSteps != 0)
  {
    backRecycleSteps = recycleSteps;
    recycleSteps = 0;
  }
}

inline void Lisp::Allocator::enableCollector()
{
  garbageSteps = backGarbageSteps;
}

inline void Lisp::Allocator::enableRecycling()
{
  recycleSteps = backRecycleSteps;
}

inline std::size_t Lisp::Allocator::getCycles() const
{
  return cycles;
}

inline unsigned short Lisp::Allocator::getGarbageSteps() const
{
  if(garbageSteps == 0)
  {
    return backGarbageSteps;
  }
  else
  {
    return garbageSteps;
  }
}

inline unsigned short Lisp::Allocator::getRecycleSteps() const
{
  if(recycleSteps == 0)
  {
    return backRecycleSteps;
  }
  else
  {
    return recycleSteps;
  }
}

inline void Lisp::Allocator::setGarbageSteps(unsigned short steps)
{
  if(garbageSteps == 0)
  {
    backGarbageSteps = steps;
  }
  else
  {
    garbageSteps = steps;
  }

}

inline void Lisp::Allocator::setRecycleSteps(unsigned short steps)
{
  if(recycleSteps == 0)
  {
    backRecycleSteps = steps;
  }
  else
  {
    recycleSteps = steps;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Processing
//
////////////////////////////////////////////////////////////////////////////////

inline void Lisp::Allocator::step()
{
  for(unsigned short i=0; i < garbageSteps; i++)
  {
    bool swapable = true;
    swapable &= consMap.step();
    swapable &= containerMap.step();
    if(swapable)
    {
      cycles++;
      consMap.swap();
      containerMap.swap();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// test and debug
//
////////////////////////////////////////////////////////////////////////////////
inline bool Lisp::Allocator::checkSanity() const
{
  return checkRootSanity() && checkBulkSanity();
}

inline bool Lisp::Allocator::checkRootSanity() const
{
  return
    checkRootSanity(Color::White) &&
    checkRootSanity(Color::Grey) &&
    checkRootSanity(Color::Black);
}

inline bool Lisp::Allocator::checkBulkSanity() const
{
  return
    checkBulkSanity(Color::White) &&
    checkBulkSanity(Color::Grey) &&
    checkBulkSanity(Color::Black);
}

inline bool Lisp::Allocator::checkSanity(Color color) const
{
  return checkRootSanity(color) && checkBulkSanity(color);
}

inline bool Lisp::Allocator::checkRootSanity(Color color) const
{
  return checkSanity(color, true);
}

inline bool Lisp::Allocator::checkBulkSanity(Color color) const
{
  return checkSanity(color, false);
}

inline bool Lisp::Allocator::checkSanity(Color color, bool root) const
{
  typedef void(Allocator::*Function)(Color color,
                                            std::function<void(const Cell &)> func) const;
  Function f;
  if(root)
  {
    f = &Allocator::forEachRootCollectible;
  }
  else
  {
    f = &Allocator::forEachBulkCollectible;
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


