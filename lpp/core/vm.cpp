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
#include <lpp/core/env.h>
#include <lpp/core/opcode.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/form.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/types/continuation.h>
#include <lpp/core/exception.h>
#include "config.h"


#ifdef NDEBUG
const bool Lisp::Vm::withDebug = false;
#else
const bool Lisp::Vm::withDebug = true;
#endif

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Cell = Lisp::Cell;
using Env = Lisp::Env;

using Function = Lisp::Function;
using Cons = Lisp::Cons;
using Symbol = Lisp::Symbol;
using Reference = Lisp::Reference;

Vm::Vm(std::shared_ptr<Allocator> _alloc,
       std::shared_ptr<Env> _env)
  : alloc(_alloc ? _alloc : std::make_shared<Allocator>()),
    env(_env ? _env : std::make_shared<Env>())
{
  dataStack.reserve(1024);
}

Lisp::Object Lisp::Vm::reference(const Cell & car, const Cell & value)
{
  return alloc->makeRoot<Reference>(car, value);
}

void Lisp::Vm::define(const std::string & name, const Object & rhs)
{
  env->set(alloc->makeRoot<Symbol>(name), rhs);
}

Object Lisp::Vm::find(const std::string & name) const
{
  return env->find(alloc->makeRoot<Symbol>(name));
}

Object Lisp::Vm::eval(const Cell & func)
{
  assert(func.isA<Function>());
  Object cont = make<Continuation>(func.as<Function>());
  return Object(cont.as<Continuation>()->eval());
}



