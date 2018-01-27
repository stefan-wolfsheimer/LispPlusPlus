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
#include "lisp_cons.h"

using Cons = Lisp::Cons;

Lisp::ConsFactory::ConsFactory(std::size_t _pageSize,
                               unsigned short _garbageSteps,
                               unsigned short _recycleSteps) :
   garbageSteps(_garbageSteps),
   recycleSteps(_recycleSteps),
   pageSize(_pageSize),
   fromColor(Color::White),
   toColor(Color::Black),
   fromRootColor(Color::WhiteRoot),
   toRootColor(Color::BlackRoot)
{
}

Lisp::ConsFactory::~ConsFactory()
{
  for(auto ptr : pages)
  {
    for(std::size_t i = 0; i < pageSize; i++)
    {
      ptr[i].unsetCar();
      ptr[i].unsetCdr();
    }
    delete [] ptr;
  }
}

Lisp::ConsFactory::Color Lisp::ConsFactory::getFromColor() const
{
  return fromColor;
}

Lisp::ConsFactory::Color Lisp::ConsFactory::getToColor() const
{
  return toColor;
}

Lisp::ConsFactory::Color Lisp::ConsFactory::getFromRootColor() const
{
  return fromRootColor;
}

Lisp::ConsFactory::Color Lisp::ConsFactory::getToRootColor() const
{
  return toRootColor;
}

void Lisp::ConsFactory::removeFromVector(Cons * cons)
{
  std::vector<Cons*> & _conses(conses[(unsigned char)cons->color]);
  assert(_conses.size() > 0);
  assert(_conses[cons->index] == cons);
  _conses[cons->index] = _conses.back();
  _conses.back()->index = cons->index;
  _conses.pop_back();
}

void Lisp::ConsFactory::addToVector(Color color, Cons * cons)
{
  cons->index = conses[(unsigned char)color].size();
  cons->color = color;
  conses[(unsigned char)color].push_back(cons);
}

void Lisp::ConsFactory::moveAllFromVectorToOther(Color colorFrom, Color colorTo)
{
  std::vector<Cons*> & vfrom(conses[(unsigned char)colorFrom]);
  std::vector<Cons*> & vto(conses[(unsigned char)colorTo]);
  for(auto cons : vfrom)
  {
    cons->color = colorTo;
  }
  vto.insert(vto.end(), vfrom.begin(), vfrom.end());
  vfrom.clear();
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
    removeFromVector(cons);
    addToVector(Color::Grey, cons);
  }
  else if(cons->getColor() == fromRootColor)
  {
    removeFromVector(cons);
    addToVector(Color::GreyRoot, cons);
  }
}

void Lisp::ConsFactory::greyChild(Cons * cons)
{
  greyChildInternal(cons);
}

Lisp::Cons * Lisp::ConsFactory::make(const Object & car,
                                     const Object & cdr)
{
  stepGargabeCollector();
  stepRecycle();
  Cons * ret;
  std::vector<Cons*> & _conses(conses[(unsigned char)Color::Void]);
  if(_conses.empty())
  {
    _conses.reserve(pageSize);
    ret = new Cons[pageSize];
    pages.push_back(ret);
    for(std::size_t i = 1; i < pageSize; i++)
    {
      _conses.push_back(&ret[i]);
    }
  }
  else
  {
    ret = _conses.back();
    _conses.pop_back();
  }
  ret->car = car;
  ret->cdr = cdr;
  ret->consFactory = this;
  ret->refCount = 1;

  // new cons is root with from-color
  addToVector((toColor == Cons::Color::Black ? Cons::Color::WhiteRoot : Cons::Color::BlackRoot),
              ret);
  return ret;
}

void Lisp::ConsFactory::root(Cons * cons)
{
  assert(!cons->isRoot());
  assert(cons->color != Cons::Color::Void);
  assert(cons == &*conses[(unsigned char)cons->color][cons->index]);
  removeFromVector(cons);
  cons->refCount = 1u;
  // new root with from-color
  addToVector((toColor == Cons::Color::Black ? Cons::Color::WhiteRoot : Cons::Color::BlackRoot),
              cons);
  stepGargabeCollector();
  stepRecycle();
}

void Lisp::ConsFactory::unroot(Cons * cons)
{
  assert(cons->isRoot());
  assert(cons == &*conses[(unsigned char)cons->color][cons->index]);
  removeFromVector(cons);
  //@todo fix this: keep color, but move from root to non-root
  //addToVector(toColor, cons);
  addToVector(Color((unsigned char)cons->color - 3), cons);
  stepGargabeCollector();
  stepRecycle();
}

std::size_t Lisp::ConsFactory::numConses(Color color) const
{
  if(color == Color::Free)
  {
    std::size_t n = 0;
    for(auto & v : freeConses)
    {
      n+= v.size();
    }
    return n;
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


std::vector<const Cons*> Lisp::ConsFactory::getConses(Color color) const
{
  std::vector<const Cons*> ret;
  if(color == Color::Free)
  {
    for(auto & v : freeConses)
    {
      ret.insert(ret.end(), v.begin(), v.end());
    }
  }
  else
  {
    for(const Cons * v: conses[(unsigned char)color])
    {
      ret.push_back(v);
    }
  }
  return ret;
}

std::vector<const Cons*> Lisp::ConsFactory::getConses(Color begin,
                                                      Color end) const
{
  std::vector<const Cons*> ret;
  for(unsigned int curr = (unsigned int)begin;
      curr < (unsigned int) end;
      ++curr)
  {
    auto tmp = getConses((Color)curr);
    ret.insert(ret.end(), tmp.begin(), tmp.end());
  }
  return ret;
}

std::vector<const Cons*> Lisp::ConsFactory::getRootConses() const
{
  return getConses(Lisp::Cons::Color::WhiteRoot, Lisp::Cons::Color::Free);
}

template<typename T> std::unordered_set<T*>
Lisp::ConsFactory::getReachableConsesAsSetIntneral() const
{
  typedef T ConsType;
  
  std::unordered_set<ConsType*> todo;
  std::unordered_set<ConsType*> root;
  todo.insert(conses[(unsigned char)Color::BlackRoot].begin(),
              conses[(unsigned char)Color::BlackRoot].end());
  todo.insert(conses[(unsigned char)Color::GreyRoot].begin(),
              conses[(unsigned char)Color::GreyRoot].end());
  todo.insert(conses[(unsigned char)Color::WhiteRoot].begin(),
              conses[(unsigned char)Color::WhiteRoot].end());
  while(!todo.empty())
  {
    ConsType * cons = *todo.begin();
    todo.erase(cons);
    root.insert(cons);
    {
      const Object & ocar(cons->car);
      ConsType * car = ocar.as<ConsType>();
      if(car && todo.find(car) == todo.end() && root.find(car) == root.end())
      {
        todo.insert(car);
      }
      const Object & ocdr(cons->cdr);
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

std::vector<const Cons*> Lisp::ConsFactory::getReachableConses() const
{
  //Todo lock
  std::unordered_set<const Cons*> root = getReachableConsesAsConstSet();
  std::vector<const Cons*> ret(root.begin(), root.end());
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
  assert(pages.size() * pageSize > root.size());
  fromColor = Color::White;
  toColor = Color::Black;
  fromRootColor = Color::WhiteRoot;
  toRootColor = Color::BlackRoot;
  conses[(unsigned char)toColor].clear();
  conses[(unsigned char)toColor].reserve(root.size() - nRoot);
  conses[(unsigned char)Color::Void].clear();
  conses[(unsigned char)Color::Void].reserve(pages.size() * pageSize - root.size());
  conses[(unsigned char)Color::Grey].clear();
  moveAllFromVectorToOther(toRootColor, fromRootColor);
  conses[(unsigned char)fromColor].clear();
  for(auto & page : pages)
  {
    for(std::size_t i = 0; i < pageSize; i++)
    {
      if(page[i].getColor() != Color::WhiteRoot)
      {
        if(root.find(&page[i]) == root.end())
        {
          addToVector(Color::Void, &page[i]);
        }
        else
        {
          addToVector(toColor, &page[i]);
        }
      }
    }
  }
}

void Lisp::ConsFactory::stepGargabeCollector()
{
  //Todo lock
  for(unsigned short i=0; i < garbageSteps; i++)
  {
    if(!conses[(unsigned char)(fromRootColor)].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)fromRootColor].back();
      conses[(unsigned char)fromRootColor].pop_back();
      addToVector(toRootColor, cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else if(!conses[(unsigned char)Color::GreyRoot].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)Color::GreyRoot].back();
      conses[(unsigned char)Color::GreyRoot].pop_back();
      addToVector(toRootColor, cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else if(!conses[(unsigned char)Color::Grey].empty())
    {
      // Todo: check edge case: cons == car or cons == cdr !
      auto cons = conses[(unsigned char)Color::Grey].back();
      conses[(unsigned char)Color::Grey].pop_back();
      addToVector(toColor, cons);
      greyChildInternal(cons->getCarCell());
      greyChildInternal(cons->getCdrCell());
    }
    else
    {
      // Todo test swap
      assert(conses[(unsigned char)Color::Grey].empty());
      assert(conses[(unsigned char)Color::GreyRoot].empty());
      assert(conses[(unsigned char)fromRootColor].empty());
      freeConses.push_back(std::vector<Cons*>());
      conses[(unsigned char)fromColor].swap(freeConses.back());
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
  for(unsigned short i=0; i < recycleSteps; i++)
  {
  }
}

