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
#include <vector>
#include <unordered_set>
#include <iterator>
#include <lpp/core/types/cons.h>


namespace Lisp
{
  class ConsPages
  {
  public:
    ConsPages(std::size_t _pageSize);

    template<typename ITR>
    class IteratorAdapter
    {
    public:
      using basic_iterator = ITR;
      using difference_type = typename basic_iterator::difference_type;
      using value_type = typename basic_iterator::value_type;
      using pointer = typename basic_iterator::pointer;
      using reference = typename basic_iterator::reference;
      using iterator_category = typename basic_iterator::iterator_category;
      inline value_type operator*() const;
      inline pointer operator->() const;
      inline IteratorAdapter& operator++();
      inline IteratorAdapter  operator++(int);
      inline IteratorAdapter& operator--();
      inline IteratorAdapter  operator--(int);
      inline bool operator==(const IteratorAdapter & rhs) const;
      inline bool operator!=(const IteratorAdapter & rhs) const;
      inline bool operator<(const IteratorAdapter & rhs) const;
      inline bool operator<=(const IteratorAdapter & rhs) const;
      inline bool operator>(const IteratorAdapter & rhs) const;
      inline bool operator>=(const IteratorAdapter & rhs) const;
      inline IteratorAdapter operator-(const difference_type & pos) const;
      inline IteratorAdapter operator+(const difference_type & pos) const;
      inline difference_type operator-(const IteratorAdapter & rhs) const;
      inline value_type operator[](const difference_type & pos) const;
      inline IteratorAdapter& operator+=(const difference_type & pos);
      inline IteratorAdapter& operator-=(const difference_type & pos);
    private:
      friend class ConsPages;
      IteratorAdapter(basic_iterator itr,
                      std::size_t pos,
                      std::size_t pageSize);
      basic_iterator itr;
      std::size_t pos;
      std::size_t pageSize;
    };

    using const_iterator = IteratorAdapter<std::vector<Cons*>::const_iterator>;
    inline const_iterator cbegin();
    inline const_iterator cend();

    //todo: remove virtual
    virtual ~ConsPages();

    inline std::size_t getPageSize() const;
    inline std::size_t getNumAllocated() const;
    inline std::size_t getNumVoid() const;
    inline std::size_t getNumRecycled() const;
    inline Cons * next();
    inline void recycle(Cons * cons);
    void recycleAll(const std::unordered_set<Cons*> & reachable,
                    CollectibleContainer<Cons> & target,
                    Color ignoreColor);
  private:
    std::size_t pageSize;
    std::size_t pos;
    std::vector<Cons*> pages;
    std::vector<Cons*> recycled;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
////////////////////////////////////////////////////////////////////////////////
template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR>::IteratorAdapter(basic_iterator _itr,
                                                       std::size_t _pos,
                                                       std::size_t _pageSize) : itr(_itr), pos(_pos), pageSize(_pageSize)
{}

template<typename ITR>
typename Lisp::ConsPages::IteratorAdapter<ITR>::value_type
Lisp::ConsPages::IteratorAdapter<ITR>::operator*() const
{
  return (*itr + pos);
}

template<typename ITR>
typename Lisp::ConsPages::IteratorAdapter<ITR>::pointer
Lisp::ConsPages::IteratorAdapter<ITR>::operator->() const
{
  return &(*itr + pos);
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR>& Lisp::ConsPages::IteratorAdapter<ITR>::operator++()
{
  if(++pos  == pageSize)
  {
    pos = 0;
    ++itr;
  }
  return *this;
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR>  Lisp::ConsPages::IteratorAdapter<ITR>::operator++(int)
{
  IteratorAdapter ret(*this);
  ++*this;
  return ret;
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR>& Lisp::ConsPages::IteratorAdapter<ITR>::operator--()
{
  if(pos)
  {
    --pos;
  }
  else
  {
    pos = pageSize - 1;
    --itr;
  }
  return *this;
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR> Lisp::ConsPages::IteratorAdapter<ITR>::operator--(int)
{
  IteratorAdapter ret(*this);
  --*this;
  return ret;
}

template<typename ITR>
bool Lisp::ConsPages::IteratorAdapter<ITR>::operator==(const IteratorAdapter & rhs) const
{
  return itr == rhs.itr && pos == rhs.pos;
}

template<typename ITR>
bool Lisp::ConsPages::IteratorAdapter<ITR>::operator!=(const IteratorAdapter & rhs) const
{
  return itr != rhs.itr || pos != rhs.pos;
}

template<typename ITR>
bool Lisp::ConsPages::IteratorAdapter<ITR>::operator<(const IteratorAdapter & rhs) const
{
  return itr < rhs.itr || (itr == rhs.itr && pos < rhs.pos);
}

template<typename ITR>
bool Lisp::ConsPages::IteratorAdapter<ITR>::operator<=(const IteratorAdapter & rhs) const
{
  return itr < rhs.itr || (itr == rhs.itr && pos <= rhs.pos);
}

template<typename ITR>
bool Lisp::ConsPages::IteratorAdapter<ITR>::operator>(const IteratorAdapter & rhs) const
{
  return itr > rhs.itr || (itr == rhs.itr && pos > rhs.pos);
}

template<typename ITR>
bool Lisp::ConsPages::IteratorAdapter<ITR>::operator>=(const IteratorAdapter & rhs) const
{
  return itr > rhs.itr || (itr == rhs.itr && pos >= rhs.pos);
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR>
Lisp::ConsPages::IteratorAdapter<ITR>::operator-(const difference_type & pos) const
{
  IteratorAdapter ret(*this);
  return ret-= pos;
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR>
Lisp::ConsPages::IteratorAdapter<ITR>::operator+(const difference_type & pos) const
{
  IteratorAdapter ret(*this);
  return ret+= pos;
}

template<typename ITR>
typename Lisp::ConsPages::IteratorAdapter<ITR>::difference_type
Lisp::ConsPages::IteratorAdapter<ITR>::operator-(const IteratorAdapter & rhs) const
{
  return (itr - rhs.itr) * pageSize + pos - rhs.pos;
}

template<typename ITR>
typename Lisp::ConsPages::IteratorAdapter<ITR>::value_type
Lisp::ConsPages::IteratorAdapter<ITR>::operator[](const difference_type & p) const
{
  return *(*this + p);
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR> &
Lisp::ConsPages::IteratorAdapter<ITR>::operator+=(const difference_type & p)
{
  difference_type n = p + pos;
  itr+= n / pageSize;
  pos = n % pageSize;
  return *this;
}

template<typename ITR>
Lisp::ConsPages::IteratorAdapter<ITR> &
Lisp::ConsPages::IteratorAdapter<ITR>::operator-=(const difference_type & p)
{
  return operator+=(-p);
}

////////////////////////////////////////////////////////////////////////////////
inline Lisp::ConsPages::ConsPages(std::size_t _page_size) : pageSize(_page_size), pos(_page_size)
{
}

inline Lisp::ConsPages::const_iterator Lisp::ConsPages::cbegin()
{
  return const_iterator(pages.begin(), 0, pageSize);
}

inline Lisp::ConsPages::const_iterator Lisp::ConsPages::cend()
{
  if(pos == pageSize)
  {
    return const_iterator(pages.end(), 0, pageSize);
  }
  else
  {
    return const_iterator(pages.end() - 1, pos, pageSize);
  }
}

inline std::size_t Lisp::ConsPages::getPageSize() const
{
  return pageSize;
}

inline std::size_t Lisp::ConsPages::getNumAllocated() const
{
  return pages.size() * pageSize;
}

inline std::size_t Lisp::ConsPages::getNumVoid() const
{
  if(pages.empty())
  {
    return 0u;
  }
  else
  {
    return pageSize - pos + getNumRecycled();
  }
}

inline std::size_t Lisp::ConsPages::getNumRecycled() const
{
  return recycled.size();
}

inline Lisp::Cons * Lisp::ConsPages::next()
{
  if(recycled.empty())
  {
    if(pos == pageSize)
    {
      pages.push_back(new Cons[pageSize]);
      pos = 0;

    }
    return pages.back() + pos++;
  }
  else
  {
    Cons * ret = recycled.back();
    recycled.pop_back();
    return ret;
  }
}

inline void Lisp::ConsPages::recycle(Cons * cons)
{
  recycled.push_back(cons);
}
