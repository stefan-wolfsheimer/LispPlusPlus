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

Lisp::ConsFactory::ConsFactory(std::size_t _pageSize) :
   pageSize(_pageSize), fromColor(Color::White), toColor(Color::Black)
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

Lisp::Cons * Lisp::ConsFactory::make(const Object & car,
                                     const Object & cdr)
{
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
  ret->color = Cons::Color::Root;
  ret->refCount = 1;
  return ret;
}

void Lisp::ConsFactory::removeFromVector(Lisp::Cons * cons)
{
  std::vector<Cons*> & _conses(conses[(unsigned char)cons->color]);
  assert(_conses.size() > 0);
  assert(_conses[cons->refCount] == cons);
  _conses[cons->refCount] = _conses.back();
  _conses.back()->refCount = cons->refCount;
  _conses.pop_back();
}

void Lisp::ConsFactory::addToVector(Color color, Lisp::Cons * cons)
{
  cons->refCount = conses[(unsigned char)color].size();
  cons->color = color;
  conses[(unsigned char)color].push_back(cons);
}

void Lisp::ConsFactory::root(Cons * cons)
{
  assert(cons->color != Cons::Color::Root);
  assert(cons->color != Cons::Color::Void);
  assert(cons == &*conses[(unsigned char)cons->color][cons->refCount]);
  removeFromVector(cons);
  cons->color = Cons::Color::Root;
  cons->refCount = 1u;
  //Todo: check children!!!
}

void Lisp::ConsFactory::unroot(Cons * cons)
{
  assert(cons->color == Cons::Color::Root);
  addToVector(toColor, cons);
}

std::size_t Lisp::ConsFactory::numConses(Color color) const
{
  if(color == Color::Root)
  {
    return
      pages.size() * pageSize
      - conses[(unsigned char)Color::White].size()
      - conses[(unsigned char)Color::Black].size()
      - conses[(unsigned char)Color::Grey].size()
      - conses[(unsigned char)Color::Void].size();
  }
  else
  {
    return conses[(unsigned char)color].size();
  }
}

std::vector<Lisp::Cons*> Lisp::ConsFactory::getRootConses() const
{
  std::vector<Cons*> ret;
  for(auto p : pages)
  {
    for(std::size_t i = 0; i < pageSize; i++)
    {
      if(p[i].getColor() == Cons::Color::Root)
      {
        ret.push_back(&p[i]);
      }
    }
  }
  return ret;
}

std::vector<Lisp::Cons*> Lisp::ConsFactory::getConses(Color color) const
{
  if(color == Color::Root)
  {
    return getRootConses();
  }
  else
  {
    return conses[(unsigned char)color];
  }
}

void Lisp::ConsFactory::cycleGarbageCollector()
{
  //Todo lock
  std::unordered_set<Cons*> todo;
  std::unordered_set<Cons*> root;
  std::size_t nRoot = 0;
  for(auto & page : pages)
  {
    for(std::size_t i = 0; i < pageSize; i++)
    {
      if(page[i].getColor() == Cons::Color::Root)
      {
        todo.insert(&page[i]);
      }
    }
  }
  while(!todo.empty())
  {
    auto cons = *todo.begin();
    if(cons->getColor() == Cons::Color::Root)
    {
      nRoot++;
    }
    todo.erase(cons);
    root.insert(cons);
    if(cons->car.isA<Cons>())
    {
      auto car = cons->car.as<Cons>();
      if(todo.find(car) == todo.end() && root.find(car) == root.end())
      {
        todo.insert(car);
      }
      auto cdr = cons->cdr.as<Cons>();
      if(todo.find(cdr) == todo.end() && root.find(cdr) == root.end())
      {
        todo.insert(cdr);
      }
    }
  }
  assert(root.size() >= nRoot);
  assert(pages.size() * pageSize > root.size());
  fromColor= Color::White;
  toColor = Color::Black;
  conses[(unsigned char)toColor].clear();
  conses[(unsigned char)toColor].reserve(root.size() - nRoot);
  conses[(unsigned char)Color::Void].clear();
  conses[(unsigned char)Color::Void].reserve(pages.size() * pageSize - root.size());
  conses[(unsigned char)Color::Grey].clear();
  conses[(unsigned char)fromColor].clear();
  for(auto & page : pages)
  {
    for(std::size_t i = 0; i < pageSize; i++)
    {
      if(page[i].getColor() != Color::Root)
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

inline void Lisp::ConsFactory::recycleChild(const Cell & cell)
{
  auto cons = cell.as<Cons>();
  if(cons && cons->getColor() == fromColor)
  {
    removeFromVector(cons);
    addToVector(Color::Grey, cons);
  }
}

bool Lisp::ConsFactory::stepGargabeCollector()
{
  //Todo lock
  if(!conses[(unsigned char)Color::Grey].empty())
  {
    auto cons = conses[(unsigned char)Color::Grey].back();
    conses[(unsigned char)Color::Grey].pop_back();
    recycleChild(cons->getCarCell());
    recycleChild(cons->getCdrCell());
    removeFromVector(cons);
    addToVector(toColor, cons);
    return false;
  }
  //Todo swap
  return true;
}

