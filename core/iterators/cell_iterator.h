#pragma once
#include <iterator>
#include <core/lisp_cell.h>

namespace Lisp
{
  class Cons;

  class CellIteratorAdapterBase
  {
  public:
    virtual ~CellIteratorAdapter() {}
  };

  template<typename ITR>
  class CellIteratorAdapter : public CellIteratorAdapterBase
  {
  };

  class CellIterator
    : public ::std::iterator<::std::random_access_iterator_tag, const Lisp::Cell>
  {
  public:
    using iterator_type = ::std::iterator<::std::random_access_iterator_tag, Lisp::Cell>;
    using difference_type = typename iterator_type::difference_type;
    using value_type = typename iterator_type::value_type;
    using pointer = typename iterator_type::pointer;
    using reference = typename iterator_type::reference;
    using iterator_category = typename iterator_type::iterator_category;

    
    inline const Cell & operator&() const;
    inline const Cell * operator->() const;
    inline ConsConstIterator& operator++();
    inline ConsConstIterator  operator++(int);
    inline ConsConstIterator& operator--();
    inline ConsConstIterator  operator--(int);
    inline bool operator==(const ConsConstIterator & rhs) const;
    inline bool operator!=(const ConsConstIterator & rhs) const;
    inline bool operator<(const ConsConstIterator & rhs) const;
    inline bool operator<=(const ConsConstIterator & rhs) const;
    inline bool operator>(const ConsConstIterator & rhs) const;
    inline bool operator>=(const ConsConstIterator & rhs) const;
    inline difference_type operator-(const ConsConstIterator & rhs) const;
    inline const Cell & operator[](const difference_type & pos) const;
    inline const Cell & operator+=(const difference_type & pos) const;
    inline const Cell & operator-=(const difference_type & pos) const;
  private:
    const Cons * cons;
    unsigned short pos;
  };
}

inline Lisp::ConsConstIterator::ConsConstIterator(const Lisp::Cons * _cons) :
  cons(_cons), pos(0)
{
}

inline const Lisp::Cell& Lisp::ConsConstIterator::operator&() const
{
  switch(pos)
  {
  case 0:
    return cons->getCarCell();
  case 1:
    return cons->getCdrCell();
  default:
    return Lisp::nil;
  }
}

inline const Lisp::Cell * Lisp::ConsConstIterator::operator->() const
{
  switch(pos)
  {
  case 0:
    return &cons->getCarCell();
  case 1:
    return &cons->getCdrCell();
  default:
    return &Lisp::nil;
  }
}

inline Lisp::ConsConstIterator& Lisp::ConsConstIterator::operator++()
{
  pos++;
  return *this;
}

inline Lisp::ConsConstIterator  Lisp::ConsConstIterator::operator++(int)
{
  ConsConstIterator self(*this);
  pos++;
  return self;
}

inline Lisp::ConsConstIterator& Lisp::ConsConstIterator::operator--()
{
  pos--;
  return *this;
}

inline Lisp::ConsConstIterator  Lisp::ConsConstIterator::operator--(int)
{
  ConsConstIterator self(*this);
  pos--;
  return self;
}

inline bool Lisp::ConsConstIterator::operator==(const Lisp::ConsConstIterator & rhs) const
{
  return cons == rhs.cons && pos == rhs.pos;
}

inline bool Lisp::ConsConstIterator::operator!=(const Lisp::ConsConstIterator & rhs) const
{
  return cons != rhs.cons || pos != rhs.pos;
}


