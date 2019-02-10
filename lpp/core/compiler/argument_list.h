#pragma once
#include <assert.h>
#include <unordered_map>
#include <memory>
#include <lpp/core/cell.h>
#include <lpp/core/types/symbol.h>

namespace Lisp
{
  class ArgumentReference
  {
  public:
    ArgumentReference(std::size_t _pos=0);
    inline std::size_t getPos() const;
  private:
    std::size_t pos;
  };

  class ArgumentList
  {
  public:
    ArgumentList(std::shared_ptr<ArgumentList> & _parent);
    inline const std::shared_ptr<ArgumentList> & getParent() const;
    inline bool isSet(const Cell & cell) const;
    inline void set(const Cell & sym, const ArgumentReference & ref);
    inline const ArgumentReference & find(const Cell & symb) const;
  private:
    struct Hash
    {
      inline std::size_t operator()(const Cell & object) const;
    };

    struct Equal
    {
      inline bool operator()(const Cell & lhs,
                             const Cell & rhs) const;
    };
    std::shared_ptr<ArgumentList> parent;
    std::unordered_map<Cell, ArgumentReference, Hash, Equal> bindings;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
inline Lisp::ArgumentReference::ArgumentReference(std::size_t _pos) : pos(_pos)
{
}

inline std::size_t Lisp::ArgumentReference::getPos() const
{
  return pos;
}

inline Lisp::ArgumentList::ArgumentList(std::shared_ptr<ArgumentList> & _parent)
  : parent(_parent)
{
}

inline const std::shared_ptr<Lisp::ArgumentList> & Lisp::ArgumentList::getParent() const
{
  return parent;
}

inline bool Lisp::ArgumentList::isSet(const Cell & cell) const
{
  assert(cell.isA<Symbol>());
  return bindings.find(cell) != bindings.end();
}

inline void Lisp::ArgumentList::set(const Cell & symb, const ArgumentReference & ref)
{
  bindings[symb] = ref;
}

inline const Lisp::ArgumentReference & Lisp::ArgumentList::find(const Cell & symb) const
{
  assert(bindings.find(symb) != bindings.end());
  assert(symb.isA<Symbol>());
  return bindings.find(symb)->second;
}

inline std::size_t Lisp::ArgumentList::Hash::operator()(const Cell & object) const
{
  static std::hash<Symbol*> symbolHash;
  assert(object.isA<Symbol>());
  return symbolHash(object.as<Symbol>());
}

inline bool Lisp::ArgumentList::Equal::operator()(const Cell & lhs, const Cell & rhs) const
{
  static std::equal_to<Symbol*> symbolEq;
  assert(lhs.isA<Symbol>());
  assert(rhs.isA<Symbol>());
  return symbolEq(lhs.as<Symbol>(), rhs.as<Symbol>());
}
