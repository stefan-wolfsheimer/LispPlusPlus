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

    inline void set(std::size_t pos, const Object & rhs);
    inline void set(std::size_t pos, Object && rhs);

    inline void append();
    inline void append(const Object & rhs);

    template<typename... ARGS>
    inline void append(const Object & a, ARGS... rest);

    virtual void forEachChild(std::function<void(const Cell&)> func) const override;
    virtual TypeId getTypeId() const override;
    virtual bool greyChildren() override;
    inline std::size_t getGcPosition() const;
    virtual void resetGcPosition() override;
    virtual bool recycleNextChild() override;
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
  return Object(data.at(pos));
}

inline Lisp::Object Lisp::Array::operator[](std::size_t pos) const
{
  return Object(data[pos]);
}

inline void Lisp::Array::set(std::size_t pos, const Object & rhs)
{
  assert(pos < data.size());
  data[pos] = rhs;
  data[pos].grey();
}

inline void Lisp::Array::set(std::size_t pos, Object && rhs)
{
  assert(pos < data.size());
  data[pos] = rhs;
  data[pos].grey();
}

inline void Lisp::Array::append()
{
}

inline void Lisp::Array::append(const Object & rhs)
{
  rhs.grey();
  data.push_back(rhs);
}

template<typename... ARGS>
void Lisp::Array::append(const Object & a, ARGS... rest)
{
  append(a);
  append(rest...);
}

inline std::size_t Lisp::Array::getGcPosition() const
{
  return gcPosition;
}

#if 0
#pragma once
#include <cstdint>
#include <vector>
#include <assert.h>
#include <lpp/core/cell.h>


namespace Lisp
{
  class Cons;

  class Array : public Container
  {
  public:
    using Color = Lisp::Color;


    //inline Color getColor() const;
    inline std::size_t getGcTop() const;
  private:
    friend class GarbageCollector;
    //Array(GarbageCollector * _consFactory, Color color, std::size_t _index);
    std::vector<Lisp::Cell> data;
    std::size_t gcTop;
    //GarbageCollector * consFactory;
    //Color color;
    //std::size_t index;
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
//inline Lisp::Array::Array(GarbageCollector * _consFactory,
//                          Color _color,
//                          std::size_t _index)
//  : consFactory(_consFactory), color(_color), index(_index)
//{
//  gcTop = 0;
//}


inline void Lisp::Array::push_back(const Cell & rhs)
{
  /*if(rhs.isA<Lisp::Cons>())
  {
    rhs.as<Cons>()->root();
    if(color == consFactory->getToRootColor())
    {
      consFactory->gcStep(rhs.as<Cons>());
    }
    }*/
  data.push_back(rhs);
}

inline void Lisp::Array::push_back(Cell && rhs)
{
  /*if(rhs.isA<Lisp::Cons>())
  {
    rhs.as<Cons>()->root();
    if(color == consFactory->getToRootColor())
    {
      consFactory->gcStep(rhs.as<Cons>());
    }
    }*/
  data.push_back(rhs);
}


//inline Lisp::Color Lisp::Array::getColor() const
//{
//  return color;
//}

inline std::size_t Lisp::Array::getGcTop() const
{
  return gcTop;
}
#endif
