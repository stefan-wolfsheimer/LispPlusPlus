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
#include "lisp_cons_factory.h"
#include "lisp_cons.h"

Lisp::ConsFactory::ConsFactory(std::size_t _pageSize)
  : code2color { Lisp::Cons::Color::Void,
                 Lisp::Cons::Color::White,
                 Lisp::Cons::Color::Grey,
                 Lisp::Cons::Color::Black,
                 Lisp::Cons::Color::Root },
    color2code {0, 1, 2, 3, 4},
    pageSize(_pageSize),
    whiteTop(0),
    greyTop(0)
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

void Lisp::ConsFactory::initChild(Object & obj, const Object & rhs)
{
  obj = rhs;
  if(Lisp::Cons * cons = rhs.as<Cons>())
  {
    if(cons->colorIndex == color2code[(unsigned char)Cons::Color::White])
    {
      // @todo move white cons to grey      
    }
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
  initChild(ret->car, car);
  initChild(ret->cdr, cdr);
  ret->consFactory = this;
  ret->colorIndex = color2code[(unsigned char)Cons::Color::Root];
  ret->refCount = 1;
  return ret;
}

void Lisp::ConsFactory::unroot(Cons * cons)
{
  cons->refCount = conses.size();
  cons->colorIndex = color2code[(unsigned char)Cons::Color::Black];
  conses.push_back(cons);
}

std::size_t Lisp::ConsFactory::numConses(Color color) const
{
  switch(color)
  {
  case Color::White:
    return whiteTop;
  case Color::Grey:
    return greyTop - whiteTop;
  case Color::Black:
    return conses.size() - greyTop - whiteTop;
  case Color::Root:
    return pages.size() * pageSize - conses.size() - freeConses.size();
  case Color::Void:
    return freeConses.size();
  }
  return 0u;
}

Lisp::Cons::Color Lisp::ConsFactory::encodeColor(unsigned char code) const
{
  return code2color[code];
}

unsigned char Lisp::ConsFactory::decodeColor(Lisp::Cons::Color color) const
{
  return color2code[(unsigned char)color];
}

