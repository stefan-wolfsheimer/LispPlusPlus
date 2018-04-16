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
#include <cstdint>
#include <vector>
//#include "core/lisp_object.h"
#include "core/lisp_cons_factory.h"


namespace Lisp
{
  class Cons;
  class ConsFactory;
  class ConsContainer
  {
  public:
    using Color = ConsFactory::Color;
    using iterator = std::vector<Cons*>::iterator;
    using reverse_iterator = std::vector<Cons*>::reverse_iterator;
    using const_iterator = std::vector<Cons*>::const_iterator;
    using const_reverse_iterator = std::vector<Cons*>::const_reverse_iterator;
    using size_type = std::vector<Cons*>::size_type;
    using difference_type = std::vector<Cons*>::difference_type;
    using reference = std::vector<Cons*>::reference;
    using const_reference = std::vector<Cons*>::const_reference;
    using pointer = std::vector<Cons*>::pointer;
    using const_pointer = std::vector<Cons*>::const_pointer;
    inline void pushCons(Cons *);
    inline void removeCons(Cons *);
    inline Color getColor() const;
    inline iterator begin();
    inline iterator end();
    inline const_iterator cbegin() const;
    inline const_iterator cend() const;
    inline reverse_iterator rbegin();
    inline reverse_iterator rend();
    inline const_reverse_iterator crbegin() const;
    inline const_reverse_iterator crend() const;
    inline reference at(size_type pos);
    inline const_reference at(size_type pos) const;
    //inline reference operator[](size_type pos);
    inline const_reference operator[](size_type pos) const;
  private:
    friend class ConsFactory;
    ConsContainer(ConsFactory * _consFactory,
                  Color color,
                  std::size_t _index);
    std::vector<Cons*> conses;
    std::size_t gcTop;
    ConsFactory * consFactory;
    Color color;
    std::size_t index;
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
inline Lisp::ConsContainer::ConsContainer(ConsFactory * _consFactory,
                                          Color _color,
                                          std::size_t _index)
  : consFactory(_consFactory), color(_color), index(_index)
{
  gcTop = 0;
}

inline void Lisp::ConsContainer::pushCons(Cons * cons)
{
  cons->root();
  conses.push_back(cons);
}

inline void Lisp::ConsContainer::removeCons(Cons * cons)
{
}

inline Lisp::ConsFactory::Color Lisp::ConsContainer::getColor() const
{
  return color;
}

inline Lisp::ConsContainer::iterator
Lisp::ConsContainer::begin()
{
  return conses.begin();
}

inline Lisp::ConsContainer::iterator
Lisp::ConsContainer::end()
{
  return conses.end();
}

inline Lisp::ConsContainer::const_iterator
Lisp::ConsContainer::cbegin() const
{
  return conses.cbegin();
}

inline Lisp::ConsContainer::const_iterator
Lisp::ConsContainer::cend() const
{
  return conses.cend();
}

inline Lisp::ConsContainer::reverse_iterator
Lisp::ConsContainer::rbegin()
{
    return conses.rbegin();
}

inline Lisp::ConsContainer::reverse_iterator
Lisp::ConsContainer::rend()
{
  return conses.rend();
}

inline Lisp::ConsContainer::const_reverse_iterator
Lisp::ConsContainer::crbegin() const
{
  return conses.crbegin();
}

inline Lisp::ConsContainer::const_reverse_iterator
Lisp::ConsContainer::crend() const
{
  return conses.crend();
}

inline Lisp::ConsContainer::reference
Lisp::ConsContainer::at(size_type pos)
{
  return conses.at(pos);
}

inline Lisp::ConsContainer::const_reference
Lisp::ConsContainer::at(size_type pos) const
{
  return conses.at(pos);
}

inline Lisp::ConsContainer::const_reference
Lisp::ConsContainer::operator[]( size_type pos ) const
{
  return conses[pos];
}

