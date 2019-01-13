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
#include <memory>
#include <lpp/core/object.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/types/array.h>
#include <lpp/core/default_env.h>
#include "types/lisp_function_interface.h"

namespace Lisp
{
  class Vm
  {
  public:
    static const bool withDebug;
    Vm(std::shared_ptr<GarbageCollector> _gc = nullptr,
       std::shared_ptr<SymbolContainer> _sc = nullptr,
       std::shared_ptr<Env> _env = nullptr);
    
    inline std::shared_ptr<GarbageCollector> getConsFactory() const;
    inline Object cons(const Object & car, const Object & cdr);
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
    
    Object symbol(const std::string & name);

    void define(const std::string & name, const Object & rhs);
    //void setq(const std::string & name, const Object & obj);
    //void setq(const Object & obj, const Object & rhs);
    //void setq(Symbol * obj, const Object & rhs);

    inline void push(const Object & rhs);
    inline void push(Object && rhs);

    inline void pop();
    inline void pop(std::size_t n);

    Object compile(const Object & obj) const;
    void eval(const Function * func);
    inline const Object & getValue() const;
  private:
    std::shared_ptr<GarbageCollector> gc;
    std::shared_ptr<SymbolContainer> sc;
    std::shared_ptr<Env> env;
    std::vector<Object> dataStack;
    std::vector<Object> values;
  };
}

/******************************************************************************
 * implementation
 ******************************************************************************/
std::shared_ptr<Lisp::GarbageCollector> Lisp::Vm::getConsFactory() const
{
  return gc;
}

inline Lisp::Object Lisp::Vm::cons(const Lisp::Object & _car,
                                   const Lisp::Object & _cdr)
{
  return Lisp::Object(gc->makeRoot<Cons>(_car, _cdr));
}

inline Lisp::Object Lisp::Vm::list()
{
  return Lisp::nil;
}

inline Lisp::Object Lisp::Vm::list(const Object & a)
{
  return cons(a, Lisp::nil);
}

inline Lisp::Object Lisp::Vm::list(Object && a)
{
  return cons(a, Lisp::nil);
}

template<typename... ARGS>
Lisp::Object Lisp::Vm::list(const Lisp::Object & a, const ARGS & ... rest)
{
  return cons(a, std::move(list(rest...)));
}

template<typename... ARGS>
Lisp::Object Lisp::Vm::list(Lisp::Object && a, const ARGS & ... rest)
{
  return cons(a, std::move(list(rest...)));
}

inline Lisp::Object Lisp::Vm::array()
{
  return gc->makeRoot<Lisp::Array>();
}

template<typename... ARGS>
inline Lisp::Object Lisp::Vm::array(const ARGS & ... rest)
{
  Object ret(gc->makeRoot<Lisp::Array>());
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

inline void Lisp::Vm::pop()
{
  dataStack.pop_back();
}

inline void Lisp::Vm::pop(std::size_t n)
{
  //dataStack.erase(dataStack.end()-n, dataStack.end());
}

inline const Lisp::Object & Lisp::Vm::getValue() const
{
  return values.front();
}

