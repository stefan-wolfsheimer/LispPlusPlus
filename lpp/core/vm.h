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
#include <memory>
#include <lpp/core/object.h>
#include <lpp/core/language_interface.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/types/array.h>

namespace Lisp
{
  class Vm
  {
  public:
    static const bool withDebug;

    Vm(std::shared_ptr<Allocator> _alloc = nullptr,
       std::shared_ptr<Env> _env = nullptr);
    
    inline std::shared_ptr<Allocator> getAllocator() const;

    template<typename T, typename... ARGS>
    inline Object make(const ARGS & ...rest);

    //inline Object cons(const Object & car, const Object & cdr);
    inline Object list();
    inline Object list(const Object & a);
    inline Object list(Object && a);

    template<typename... ARGS>
    Object list(const Object & a, const ARGS & ... rest);

    template<typename... ARGS>
    Object list(Object && a, const ARGS & ... rest);

    inline Object array();

    template<typename... ARGS>
    inline Object array(const ARGS & ... rest);

    /**
     * Create a reference to object.
     * Usually car is the symbol representing the reference.
     */
    Object reference(const Cell & car, const Cell & value);

    void define(const std::string & name, const Object & rhs);
    Object find(const std::string & name) const;

    inline void push(const Object & rhs);
    inline void push(Object && rhs);

    inline Object top() const;
    inline void pop();
    inline void pop(std::size_t n);
    inline std::size_t stackSize() const;

    inline Object compile(const LanguageInterface * lang, const Cell & cell) const;
    Object compileAndEval(const LanguageInterface * lang, const Cell & cell);
    Object compile(const Cell & lang, const Cell & cell) const;
    Object compileAndEval(const Cell & lang, const Cell & cell); 

    Object evalAndReturn(Function * func);
    void eval(Function * func);

  private:
    template<typename C>
    inline const C & _makeRoot(std::true_type, const C & c);

    template<typename C, typename... ARGS>
    inline C * _makeRoot(std::false_type, const ARGS & ... rest);

    std::shared_ptr<Allocator> alloc;
    std::shared_ptr<Env> env;
    std::vector<Object> dataStack;
  };
}

/******************************************************************************
 * implementation
 ******************************************************************************/
std::shared_ptr<Lisp::Allocator> Lisp::Vm::getAllocator() const
{
  return alloc;
}

template<typename T, typename... ARGS>
inline Lisp::Object Lisp::Vm::make(const ARGS & ...rest)
{
  return Lisp::Object(_makeRoot<T>(typename TypeTraits<T>::IsAtomic(),  rest...));
}

inline Lisp::Object Lisp::Vm::list()
{
  return Lisp::nil;
}

inline Lisp::Object Lisp::Vm::list(const Object & a)
{
  return make<Cons>(a, Lisp::nil);
}

inline Lisp::Object Lisp::Vm::list(Object && a)
{
  return make<Cons>(a, Lisp::nil);
}

template<typename... ARGS>
Lisp::Object Lisp::Vm::list(const Lisp::Object & a, const ARGS & ... rest)
{
  return make<Cons>(a, std::move(list(rest...)));
}

template<typename... ARGS>
Lisp::Object Lisp::Vm::list(Lisp::Object && a, const ARGS & ... rest)
{
  return make<Cons>(a, std::move(list(rest...)));
}

inline Lisp::Object Lisp::Vm::array()
{
  return alloc->makeRoot<Lisp::Array>();
}

template<typename... ARGS>
inline Lisp::Object Lisp::Vm::array(const ARGS & ... rest)
{
  Object ret(alloc->makeRoot<Lisp::Array>());
  ret.as<Array>()->append(rest...);
  return ret;
}

inline void Lisp::Vm::push(const Object & rhs)
{
  dataStack.emplace_back(rhs);
}

inline void Lisp::Vm::push(Object && rhs)
{
  dataStack.emplace_back(std::move(rhs));
}

inline Lisp::Object Lisp::Vm::top() const
{
  return dataStack.back();
}

inline void Lisp::Vm::pop()
{
  dataStack.pop_back();
}

inline void Lisp::Vm::pop(std::size_t n)
{
  for(std::size_t i = 0; i < n; i++)
  {
    pop();
  }
}

inline std::size_t Lisp::Vm::stackSize() const
{
  return dataStack.size();
}

template<typename C>
inline const C & Lisp::Vm::_makeRoot(std::true_type, const C & c)
{
  return c;
}

template<typename T, typename... ARGS>
inline T * Lisp::Vm::_makeRoot(std::false_type, const ARGS & ... rest)
{
  return alloc->makeRoot<T>(rest...);
}

inline Lisp::Object Lisp::Vm::compile(const Lisp::LanguageInterface * lang, const Lisp::Cell & cell) const
{
  return lang->compile(cell);
}

