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
#include <assert.h>
#include <lpp/core/vm.h>
#include <lpp/core/default_env.h>
#include <lpp/core/env.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/opcode.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/symbol.h>

#include "config.h"


#ifdef NDEBUG
const bool Lisp::Vm::withDebug = false;
#else
const bool Lisp::Vm::withDebug = true;
#endif

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Function = Lisp::Function;
using Cons = Lisp::Cons;
using Symbol = Lisp::Symbol;
using Env = Lisp::Env;

Vm::Vm(std::shared_ptr<GarbageCollector> _gc,
       std::shared_ptr<SymbolContainer> _sc,
       std::shared_ptr<Env> _env)
  : gc(_gc ? _gc : std::make_shared<GarbageCollector>()),
    sc(_sc ? _sc : std::make_shared<SymbolContainer>()),
    env(_env ? _env : makeDefaultEnv(gc, sc))
{
  dataStack.reserve(1024);
  values.reserve(1024);
  values.push_back(Lisp::nil);
}

Lisp::Object Lisp::Vm::symbol(const std::string & name)
{
  return sc->make(name);
}

void Lisp::Vm::define(const std::string & name, const Object & rhs)
{
  env->set(sc->make(name), rhs);
}

Object Lisp::Vm::find(const std::string & name) const
{
  return env->find(sc->make(name));
}

Object Vm::compile(const Object & obj) const
{
  Jit jit(gc, sc, env);
  jit.pass1(obj);
  jit.pass2(obj);
  return jit.function;
}

void Lisp::Vm::eval(const Function * func)
{
  std::size_t i;
  auto itr = func->instructions.begin();
  auto end = func->instructions.end();
  while(itr != end)
  {
    switch(*itr)
    {
    case SETV:
      ++itr;
      assert(itr != end);
      assert(*itr < func->data.size());
      values.resize(1);
      values[0] = func->data.atCell(*itr);
      break;

    case LOOKUP:
      ++itr;
      assert(itr != end);
      assert(*itr < func->data.size());
      assert(func->data.atCell(*itr).isA<Symbol>());
      values.resize(1);
      // @todo throw if undefined
      values[0] = env->find(func->data.atCell(*itr));
      break;

    case DEFINEV:
      ++itr;
      assert(itr != end);
      assert(*itr < func->data.size());
      assert(func->data.atCell(*itr).isA<Symbol>());
      env->set(func->data.atCell(*itr), values[0]);
      break;
    }
    ++itr;
  }
}



