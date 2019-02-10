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
#include <lpp/core/object.h>
#include <lpp/core/types/container.h>

namespace Lisp
{
  class Array : public Container
  {
  public:
    template<typename... ARGS>
    Array(ARGS... rest);

    inline std::size_t size() const;
    inline const Cell & atCell(std::size_t pos) const;
    inline Object at(std::size_t pos) const;
    inline Object operator[](std::size_t pos) const;

    inline void set(std::size_t pos, const Cell & rhs);
    inline void set(std::size_t pos, Cell && rhs);

    inline void append();
    inline void append(const Cell & rhs);

    template<typename... ARGS>
    inline void append(const Cell & a, ARGS... rest);
    inline std::size_t getGcPosition() const;
    inline void reserve(std::size_t s);
    inline void shrink();

    //////////////////////////////////////////////////
    // implementation of the Container interface
    //////////////////////////////////////////////////
    virtual void forEachChild(std::function<void(const Cell&)> func) const override
    {
      forEachChildImpl(func);
    }

    virtual TypeId getTypeId() const override
    {
      return getTypeIdImpl();
    }

    virtual bool greyChildren() override
    {
      return greyChildrenImpl();
    }

    virtual void resetGcPosition() override
    {
      return resetGcPositionImpl();
    }

    virtual bool recycleNextChild() override
    {
      return recycleNextChildImpl();
    }

    inline void forEachChildImpl(std::function<void(const Cell&)> func) const;
    inline TypeId getTypeIdImpl() const;
    inline bool greyChildrenImpl();
    inline std::size_t getGcPositionImpl() const;
    inline void resetGcPositionImpl();
    inline bool recycleNextChildImpl();
  private:
    std::size_t gcPosition;
    std::vector<Lisp::Cell> data;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
template<typename... ARGS>
inline Lisp::Array::Array(ARGS... rest)
{
  gcPosition = 0u;
  append(rest...);
}
  
inline std::size_t Lisp::Array::size() const
{
  return data.size();
}

inline const Lisp::Cell & Lisp::Array::atCell(std::size_t pos) const
{
  return data.at(pos);
}

inline Lisp::Object Lisp::Array::at(std::size_t pos) const
{
  return Object(atCell(pos));
}

inline Lisp::Object Lisp::Array::operator[](std::size_t pos) const
{
  return Object(data[pos]);
}

inline void Lisp::Array::set(std::size_t pos, const Cell & rhs)
{
  assert(pos < data.size());
  data[pos] = rhs;
  data[pos].grey();
}

inline void Lisp::Array::set(std::size_t pos, Cell && rhs)
{
  assert(pos < data.size());
  data[pos] = rhs;
  data[pos].grey();
}

inline void Lisp::Array::append()
{
}

inline void Lisp::Array::append(const Cell & rhs)
{
  rhs.grey();
  data.push_back(rhs);
}

template<typename... ARGS>
void Lisp::Array::append(const Cell & a, ARGS... rest)
{
  append(a);
  append(rest...);
}

inline void Lisp::Array::reserve(std::size_t s)
{
  data.reserve(s);
}

inline void Lisp::Array::shrink()
{
  data.shrink_to_fit();
}

inline std::size_t Lisp::Array::getGcPosition() const
{
  return gcPosition;
}

inline void Lisp::Array::forEachChildImpl(std::function<void(const Cell&)> func) const
{
  for(const Cell & c : data)
  {
    func(c);
  }
}

Lisp::TypeId Lisp::Array::getTypeIdImpl() const
{
  return TypeTraits<Array>::getTypeId();
}

bool Lisp::Array::greyChildrenImpl()
{
  if(gcPosition < data.size())
  {
    data[gcPosition].grey();
    if(++gcPosition == data.size())
    {
      gcPosition = 0;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}

void Lisp::Array::resetGcPositionImpl()
{
  gcPosition = 0;
}

bool Lisp::Array::recycleNextChildImpl()
{
  if(gcPosition < data.size())
  {
    if(!data[gcPosition].isA<Collectible>())
    {
      data[gcPosition] = Lisp::nil;
    }
    return ++gcPosition == data.size();
  }
  return true;
}
