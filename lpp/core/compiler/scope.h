#pragma once
#include <assert.h>
#include <unordered_map>
#include <memory>
#include <lpp/core/cell.h>
#include <lpp/core/object.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/function.h>
#include <lpp/core/exception.h>


namespace Lisp
{
  class Function;

  class Scope
  {
  public:
    Scope(std::shared_ptr<Scope> & _parent, const Object & funcObj);
    inline const std::shared_ptr<Scope> & getParent() const;

    /**
     * return true if symbol is set in top argument list.
     */
    inline bool isSet(const Cell & symbol, bool recursive=false) const;

    /**
     * Add a new argument to the list
     */
    inline void add(const Cell & symbol);
    inline std::pair<Function*, std::size_t> find(const Cell & symb, bool recursive=false) const;

    inline Function * getFunction() const;

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
    Object funcObj;
    std::shared_ptr<Scope> parent;
    std::unordered_map<Cell, std::size_t, Hash, Equal> bindings;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
inline Lisp::Scope::Scope(std::shared_ptr<Scope> & _parent, const Object & _funcObj)
  : parent(_parent), funcObj(_funcObj)
{
}

inline const std::shared_ptr<Lisp::Scope> & Lisp::Scope::getParent() const
{
  return parent;
}

inline Lisp::Function * Lisp::Scope::getFunction() const
{
  return funcObj.as<Function>();
}

inline void Lisp::Scope::add(const Cell & symbol)
{
  assert(symbol.isA<Symbol>());
  bindings.insert(std::make_pair(symbol, getFunction()->numArguments()));
  getFunction()->addArgument(symbol);
}

inline bool Lisp::Scope::isSet(const Cell & cell, bool recursive) const
{
  assert(cell.isA<Symbol>());
  if(bindings.find(cell) == bindings.end())
  {
    if(recursive && parent)
    {
      return parent->isSet(cell);
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


inline std::pair<Lisp::Function*, std::size_t> Lisp::Scope::find(const Cell & symb,
                                                                 bool recursive) const
{
  assert(symb.isA<Symbol>());
  auto itr = bindings.find(symb);
  if(itr == bindings.end())
  {
    if(recursive && parent)
    {
      return parent->find(symb, recursive);
    }
    else
    {
      return std::make_pair(nullptr, 0);
    }
  }
  else
  {
    return std::make_pair(funcObj.as<Function>(), itr->second);
  }
}

inline std::size_t Lisp::Scope::Hash::operator()(const Cell & object) const
{
  static std::hash<Symbol*> symbolHash;
  assert(object.isA<Symbol>());
  return symbolHash(object.as<Symbol>());
}

inline bool Lisp::Scope::Equal::operator()(const Cell & lhs, const Cell & rhs) const
{
  static std::equal_to<Symbol*> symbolEq;
  assert(lhs.isA<Symbol>());
  assert(rhs.isA<Symbol>());
  return symbolEq(lhs.as<Symbol>(), rhs.as<Symbol>());
}
