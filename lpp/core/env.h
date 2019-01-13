/******************************************************************************
Copyright (c) 2017-2019, Stefan Wolfsheimer

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
#include <assert.h>
#include <functional>
#include <unordered_map>
#include <lpp/core/cell.h>
#include <lpp/core/object.h>
#include <lpp/core/types/symbol.h>

namespace Lisp
{
  class Env
  {
  public:
    void set(const Cell & sym, const Object & obj);
    void set(const Cell & sym, Object && obj);
    bool unset(Symbol * symb);

    /**
     * Find the reference to the object that is assigned to the symbol.
     * If not found, Lisp::undefined is returned.
     * @todo throw exection if not found
     *
     * @returns an Object of type Lisp::Reference (a Cons-like type of the form (Symbol . Object)
     */
    inline const Object & find(const Cell & symb) const;

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

    std::unordered_map<Cell, Object, Hash, Equal> bindings;
  };

} //namespace Lisp

inline std::size_t Lisp::Env::Hash::operator()(const Cell & object) const
{
  static std::hash<Symbol*> symbolHash;
  assert(object.isA<Symbol>());
  return symbolHash(object.as<Symbol>());
}

inline bool Lisp::Env::Equal::operator()(const Cell & lhs, const Cell & rhs) const
{
  static std::equal_to<Symbol*> symbolEq;
  assert(lhs.isA<Symbol>());
  assert(rhs.isA<Symbol>());
  return symbolEq(lhs.as<Symbol>(), rhs.as<Symbol>());
}

inline void Lisp::Env::set(const Cell & symb, const Object & obj)
{
  bindings[symb] = obj;
}

inline void Lisp::Env::set(const Cell & symb, Object && obj)
{
  bindings[symb] = obj;
}

inline const Lisp::Object & Lisp::Env::find(const Cell & obj) const
{
  assert(obj.isA<Symbol>());
  auto itr = bindings.find(obj);
  if(itr == bindings.end())
  {
    return Lisp::undefined;
  }
  else
  {
    return itr->second;
  }
}
