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
#include <lpp/core/types/container.h>

namespace Lisp
{
  class Array : public Container
  {
  public:
    virtual TypeId getTypeId() const override;
    virtual bool greyChildren() override;
  };
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
    using const_iterator = std::vector<Cell>::const_iterator;
    using const_reverse_iterator = std::vector<Cell>::const_reverse_iterator;
    using size_type = std::vector<Cell>::size_type;
    using difference_type = std::vector<Cell>::difference_type;
    using reference = std::vector<Cell>::reference;
    using const_reference = std::vector<Cell>::const_reference;
    using pointer = std::vector<Cell>::pointer;
    using const_pointer = std::vector<Cell>::const_pointer;
    inline const_iterator cbegin() const;
    inline const_iterator cend() const;
    inline const_reverse_iterator crbegin() const;
    inline const_reverse_iterator crend() const;
    inline const_reference at(size_type pos) const;
    inline const_reference operator[](size_type pos) const;

    inline void push_back(const Cell & rhs);
    inline void push_back(Cell && rhs);

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

inline Lisp::Array::const_iterator Lisp::Array::cbegin() const
{
  return data.cbegin();
}

inline Lisp::Array::const_iterator Lisp::Array::cend() const
{
  return data.cend();
}

inline Lisp::Array::const_reverse_iterator Lisp::Array::crbegin() const
{
  return data.crbegin();
}

inline Lisp::Array::const_reverse_iterator Lisp::Array::crend() const
{
  return data.crend();
}

inline Lisp::Array::const_reference Lisp::Array::at(size_type pos) const
{
  return data.at(pos);
}

inline Lisp::Array::const_reference
Lisp::Array::operator[]( size_type pos ) const
{
  return data[pos];
}

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
