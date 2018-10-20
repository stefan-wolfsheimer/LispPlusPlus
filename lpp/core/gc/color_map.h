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
#include <functional>
#include <unordered_set>
#include <lpp/core/gc/color.h>
#include <lpp/core/gc/collectible_container.h>
#include <lpp/core/cell.h>

namespace Lisp
{
  class GarbageCollector;

  template<typename T>
  class ColorMap
  {
  public:
    ColorMap(GarbageCollector * _parent);
    ~ColorMap();
    inline void add(T * obj);
    inline void addRoot(T * obj);
    inline std::size_t size(Color color) const;
    inline std::size_t rootSize(Color color) const;
    inline std::size_t numDisposed() const;
    inline bool step();
    inline void swap();
    inline void swap(const std::unordered_set<T*> & tobekept);
    inline T * popDisposed();
    inline void forEachBulk(Color color,
                            std::function<void(const Cell &)> func) const;
    inline void forEachRoot(Color color,
                            std::function<void(const Cell &)> func) const;

  private:
    inline void forEach(const CollectibleContainer<T> & elements,
                        std::function<void(const Cell &)> func) const;
    GarbageCollector * parent;
    CollectibleContainer<T> * white;
    CollectibleContainer<T> * grey;
    CollectibleContainer<T> * black;
    CollectibleContainer<T> * whiteRoot;
    CollectibleContainer<T> * greyRoot;
    CollectibleContainer<T> * blackRoot;
    UnmanagedCollectibleContainer<T> disposed;
  };
}

template<typename T>
Lisp::ColorMap<T>::ColorMap(GarbageCollector * p)
  : parent(p)
{
  white     = new CollectibleContainer<T>(Lisp::Color::White, false, p);
  grey      = new CollectibleContainer<T>(Lisp::Color::Grey,  false, p);
  black     = new CollectibleContainer<T>(Lisp::Color::Black, false, p);
  whiteRoot = new CollectibleContainer<T>(Lisp::Color::White, true,  p);
  greyRoot  = new CollectibleContainer<T>(Lisp::Color::Grey,  true,  p);
  blackRoot = new CollectibleContainer<T>(Lisp::Color::Black, true,  p);

  // we don't know if another object still refers to unrooted objects
  // -> never transition from root to white
  whiteRoot->otherElements = grey;
  greyRoot->otherElements  = grey;
  blackRoot->otherElements = black;
  white->otherElements     = whiteRoot;
  grey->otherElements      = whiteRoot;
  black->otherElements     = whiteRoot;


  whiteRoot->greyElements  = greyRoot;
  greyRoot->greyElements   = nullptr;
  blackRoot->greyElements  = nullptr;
  white->greyElements      = grey;
  grey->greyElements       = nullptr;
  black->greyElements      = nullptr;
  
  whiteRoot->toElements    = greyRoot;
  greyRoot->toElements     = blackRoot;
  blackRoot->toElements    = nullptr;
  white->toElements        = grey;
  grey->toElements         = black;
  black->toElements        = nullptr;
}

template<typename T>
Lisp::ColorMap<T>::~ColorMap()
{
  delete whiteRoot;
  delete greyRoot;
  delete blackRoot;
  delete white;
  delete grey;
  delete black;
}

template<typename T>
inline void Lisp::ColorMap<T>::add(T * obj)
{
  white->add(obj);
}

template<typename T>
inline void Lisp::ColorMap<T>::addRoot(T * obj)
{
  whiteRoot->add(obj);
}

template<typename T>
inline std::size_t Lisp::ColorMap<T>::size(Color color) const
{
  switch(color)
  {
  case Color::White: return white->size();
  case Color::Grey: return grey->size();
  case Color::Black: return black->size();
  }
  return 0;
}

template<typename T>
inline std::size_t Lisp::ColorMap<T>::rootSize(Color color) const
{
  switch(color)
  {
  case Color::White: return whiteRoot->size();
  case Color::Grey: return greyRoot->size();
  case Color::Black: return blackRoot->size();
  }
  return 0; 
}

template<typename T>
inline std::size_t Lisp::ColorMap<T>::numDisposed() const
{
  return disposed.size();
}

template<typename T>
inline bool Lisp::ColorMap<T>::step()
{
  if(!whiteRoot->empty())
  {
    assert(whiteRoot->back()->getIndex() == whiteRoot->size()-1);
    T * obj = whiteRoot->back();
    if(obj->greyChildren())
    {
      assert(obj->checkIndex());
      blackRoot->move(obj);
      assert(obj->checkIndex());
    }
    return false;
  }
  else if(!greyRoot->empty())
  {
    assert(greyRoot->back()->getIndex() == greyRoot->size()-1);
    T * obj = greyRoot->back();
    if(obj->greyChildren())
    {
      assert(obj->checkIndex());
      blackRoot->move(obj);
      assert(obj->checkIndex());
    }
    return false;
  }
  else if(!grey->empty())
  {
    assert(grey->back()->getIndex() == grey->size()-1);
    T * obj = grey->back();
    if(obj->greyChildren())
    {
      assert(obj->checkIndex());
      black->move(obj);
      assert(obj->checkIndex());
    }
    return false;
  }
  else
  {
    return true;
  }
}

template<typename T>
inline void Lisp::ColorMap<T>::swap()
{
  CollectibleContainer<T> * tmp;
  assert(grey->empty());
  assert(greyRoot->empty());
  assert(whiteRoot->empty());

  disposed.move(*white);
  tmp = white;
  white = black;
  black = tmp;
  tmp = whiteRoot;
  whiteRoot = blackRoot;
  blackRoot = tmp;

  black->color = Color::Black;
  blackRoot->color = Color::Black;
  white->color = Color::White;
  whiteRoot->color = Color::White;

  whiteRoot->otherElements = grey;
  blackRoot->otherElements = black;

  whiteRoot->greyElements  = greyRoot;
  blackRoot->greyElements  = nullptr;
  white->greyElements      = grey;
  black->greyElements      = nullptr;

  white->otherElements     = whiteRoot;
  grey->otherElements      = whiteRoot;
  black->otherElements     = whiteRoot;

  whiteRoot->toElements    = greyRoot;
  greyRoot->toElements     = blackRoot;
  blackRoot->toElements    = nullptr;
  white->toElements        = grey;
  grey->toElements         = black;
  black->toElements        = nullptr;

  assert(whiteRoot->otherElements == grey);
  assert(greyRoot->otherElements == grey);
  assert(blackRoot->otherElements == black);
  assert(white->otherElements == whiteRoot);
  assert(grey->otherElements == whiteRoot);
  assert(black->otherElements == whiteRoot);
  assert(whiteRoot->greyElements == greyRoot);
  assert(greyRoot->greyElements == nullptr);
  assert(blackRoot->greyElements == nullptr);
  assert(white->greyElements == grey);
  assert(grey->greyElements == nullptr);
  assert(black->greyElements == nullptr);
  assert(whiteRoot->toElements == greyRoot);
  assert(greyRoot->toElements == blackRoot);
  assert(blackRoot->toElements == nullptr);
  assert(white->toElements == grey);
  assert(grey->toElements == black);
  assert(black->toElements == nullptr);
}

template<typename T>
inline void Lisp::ColorMap<T>::swap(const std::unordered_set<T*> & tobekept)
{
  CollectibleContainer<T> removed(Color::White, false, nullptr);
  assert(white->elements.size() +
         black->elements.size() +
         grey->elements.size() >= tobekept.size());
  removed.elements.reserve(white->elements.size() +
                           black->elements.size() +
                           grey->elements.size() - tobekept.size());
  for(auto container : {white, grey, black})
  {
    for(auto obj : container->elements)
    {
      if(tobekept.find(obj) == tobekept.end())
      {
        removed.elements.push_back(obj);
      }
    }
    container->elements.clear();
  }
  white->elements.reserve(tobekept.size());
  for(auto obj : tobekept)
  {
    white->add(obj);
  }
  whiteRoot->elements.reserve(whiteRoot->elements.size() +
                              greyRoot->elements.size() +
                              blackRoot->elements.size());
  for(auto obj : *blackRoot)
  {
    whiteRoot->add(obj);
  }
  for(auto obj : *greyRoot)
  {
    whiteRoot->add(obj);
  }
  blackRoot->elements.clear();
  greyRoot->elements.clear();
  disposed.move(removed);
}

template<typename T>
inline T * Lisp::ColorMap<T>::popDisposed()
{
  return disposed.popBack();
}

template<typename T>
inline void Lisp::ColorMap<T>::forEach(const CollectibleContainer<T> & container,
                                       std::function<void(const Cell &)> func) const
{
  for(auto itr = container.cbegin(); itr != container.cend(); ++itr)
  {
    Cell cell(*itr, (*itr)->getTypeId());
    func(Cell(*itr, (*itr)->getTypeId()));
  }
}

template<typename T>
inline void Lisp::ColorMap<T>::forEachBulk(Color color,
                                           std::function<void(const Cell &)> func) const
{
  switch(color)
  {
  case Color::White: forEach(*white, func); break;
  case Color::Grey:  forEach(*grey, func); break;
  case Color::Black: forEach(*black, func); break;
  }
}

template<typename T>
inline void Lisp::ColorMap<T>::forEachRoot(Color color,
                                           std::function<void(const Cell &)> func) const
{
  switch(color)
  {
  case Color::White: forEach(*whiteRoot, func); break;
  case Color::Grey:  forEach(*greyRoot,  func); break;
  case Color::Black: forEach(*blackRoot, func); break;
  }
}
