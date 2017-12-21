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
   pageSize(_pageSize)
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
  if(freeConses.empty())
  {
    freeConses.reserve(pageSize);
    ret = new Cons[pageSize];
    pages.push_back(ret);
    for(std::size_t i = 1; i < pageSize; i++)
    {
      freeConses.push_back(&ret[i]);
    }
  }
  else
  {
    ret = freeConses.back();
    freeConses.pop_back();
  }
  ret->car = car;
  ret->cdr = cdr;
  ret->consFactory = this;
  ret->color = Cons::Color::Root;
  ret->refCount = 1;
  return ret;
}

void Lisp::ConsFactory::removeFromVector(std::vector<Lisp::Cons*> & v,
                                         Lisp::Cons * cons)
{
  assert(v.size() > 0);
  v[cons->refCount] = v.back();
  v.back()->refCount = cons->refCount;
  v.pop_back();
}

void Lisp::ConsFactory::root(Cons * cons)
{
  switch(cons->color)
  {
  case Cons::Color::Black:
    assert(cons == &*blackConses[cons->refCount]);
    removeFromVector(blackConses, cons);
    break;
  case Cons::Color::White:
    assert(cons == &*whiteConses[cons->refCount]);
    removeFromVector(whiteConses, cons);
    break;
  case Cons::Color::Grey:
    assert(cons == &*greyConses[cons->refCount]);
    removeFromVector(greyConses, cons);
    break;
  case Cons::Color::Root:
  case Cons::Color::Void:
    assert(cons->color != Cons::Color::Root);
    assert(cons->color != Cons::Color::Void);
    break;
  }
  cons->color = Cons::Color::Root;
  cons->refCount = 1u;
}

void Lisp::ConsFactory::unroot(Cons * cons)
{
  assert(cons->color == Cons::Color::Root);
  cons->refCount = blackConses.size();
  cons->color = Cons::Color::Black;
  blackConses.push_back(cons);
}

std::size_t Lisp::ConsFactory::numConses(Color color) const
{
  switch(color)
  {
  case Color::White:
    return whiteConses.size();
  case Color::Grey:
    return greyConses.size();
  case Color::Black:
    return blackConses.size();
  case Color::Root:
    return
      pages.size() * pageSize
      - whiteConses.size()
      - blackConses.size()
      - greyConses.size()
      - freeConses.size();
  case Color::Void:
    return freeConses.size();
  }
  return 0u;
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
  switch(color)
  {
  case Color::White:
    return whiteConses;
  case Color::Grey:
    return greyConses;
  case Color::Black:
    return blackConses;
  case Color::Root:
    return getRootConses();
  case Color::Void:
    return freeConses;
  }
  return std::vector<Cons*>();
}

void Lisp::ConsFactory::cycleGarbageCollector()
{
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
  blackConses.clear();
  blackConses.reserve(root.size() - nRoot);
  freeConses.clear();
  freeConses.reserve(pages.size() * pageSize - root.size());
  greyConses.clear();
  whiteConses.clear();
  for(auto & page : pages)
  {
    for(std::size_t i = 0; i < pageSize; i++)
    {
      if(page[i].getColor() != Cons::Color::Root)
      {
        if(root.find(&page[i]) == root.end())
        {
          page[i].color = Cons::Color::Void;
          page[i].refCount = freeConses.size();
          freeConses.push_back(&page[i]);
        }
        else
        {
          page[i].color = Cons::Color::Black;
          page[i].refCount = blackConses.size();
          blackConses.push_back(&page[i]);
        }
      }
    }
  }
}

