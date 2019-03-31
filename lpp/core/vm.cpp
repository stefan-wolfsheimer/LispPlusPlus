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
#include <lpp/core/types/reference.h>
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


// logging data stack
#ifdef DO_ASM_LOG
#define LOG_DATA_STACK(STACK)  {                        \
    ASM_LOG("STACK_SIZE: " << STACK.size());            \
    for(std::size_t i = 0; i < STACK.size(); i++)       \
    {                                                   \
      ASM_LOG("STACK [" << i << "] " << STACK[i]);      \
    }                                                   \
  }
#else
#define LOG_DATA_STACK(STACK)
#endif




Vm::Vm(std::shared_ptr<Allocator> _alloc,
       std::shared_ptr<Env> _env)
  : alloc(_alloc ? _alloc : std::make_shared<Allocator>()),
    env(_env ? _env : makeDefaultEnv(alloc))
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

//@todo remove
Object Vm::compile(const Object & obj) const
{
  Jit jit(alloc, env);
  return jit.compile(obj);
}

//@todo remove
Object Vm::compileAndEval(const Object & obj)
{
  Object func = compile(obj);
  eval(func.as<Function>());
  Object ret = top();
  pop();
  return ret;
}

Object Vm::evalAndReturn(Function * func)
{
  eval(func);
  Object ret = top();
  pop();
  return ret;
}

// @todo move to separate module
struct ExecutionState
{
  Function * func;
  Function::const_iterator itr;
  Function::const_iterator end;
  std::size_t returnPos;

  ExecutionState(const ExecutionState & rhs)
    : func(rhs.func),
      itr(rhs.itr),
      end(rhs.end),
      returnPos(rhs.returnPos)
  {
  }

  ExecutionState(const ExecutionState & rhs, Function::const_iterator _itr)
    : func(rhs.func),
      itr(_itr),
      end(rhs.end),
      returnPos(rhs.returnPos)
  {
  }

  ExecutionState(Function * _func,
                 std::size_t _returnPos)
    : func(_func),
      itr(_func->cbegin()),
      end(_func->cend()),
      returnPos(_returnPos)
  {
    ASM_LOG("FUNC\t" << func <<
            " itr: " << (itr - func->cbegin()) << "/" << func->numInstructions() <<
            " nargs: " << func->numArguments() <<
            " returnPos: " << returnPos);
  }

  ExecutionState(Function * _func,
                 std::size_t _returnPos,
                 Function::const_iterator _itr)
    : func(_func),
      itr(_itr),
      end(_func->cend()),
      returnPos(_returnPos)
  {
    ASM_LOG("FUNC\t" << func <<
            " itr: " << (itr - func->cbegin()) << "/" << func->numInstructions() <<
            " nargs: " << func->numArguments() <<
            " returnPos: " << returnPos);
  }
};

void Lisp::Vm::eval(Function * __func)
{
  ASM_LOG("----------------------------------");
  ASM_LOG("eval " << __func);
  ASM_LOG("----------------------------------");
  LOG_DATA_STACK(dataStack);
  ExecutionState state(__func, dataStack.size());
  std::vector<ExecutionState> executionStack;
  std::size_t i;
  while(true)
  {
    while(state.itr != state.end)
    {
      switch(*state.itr)
      {
      case RETURNV:
        // return a value from function data
        assert((state.itr + 1) < state.end);
        assert(state.itr[1] < state.func->dataSize());
        ASM_LOG("\tRETURNV " << state.itr[1] << ": " <<
                state.func->data.atCell(state.itr[1]) <<
                " stackSize: " << dataStack.size() <<
                " returnPos: " << state.returnPos);
        if(state.returnPos < dataStack.size())
        {
          dataStack[state.returnPos] = state.func->getValue(state.itr[1]);
        }
        else
        {
          assert(state.returnPos == dataStack.size());
          dataStack.push_back(Object(state.func->getValue(state.itr[1])));
        }
        state.itr += 2;
        break;

      case RETURNS:
        // return a value from stack
        assert((state.itr + 1) < state.end);
        assert(state.itr[1] <= dataStack.size());
        ASM_LOG("\tRETURNS " << state.itr[1] << ": " <<
                *(dataStack.end() - state.itr[1]) <<
                " stackSize: " << dataStack.size() <<
                " returnPos: " << state.returnPos);
        if(dataStack.size() - state.itr[1] != state.returnPos)
        {
          dataStack[state.returnPos] = dataStack[dataStack.size() - state.itr[1]];
        }
        state.itr += 2;
        break;

      case RETURNL:
        // return a value from function data
        // @todo exception if unbound
        assert((state.itr + 1) < state.end);
        assert(state.itr[1] < state.func->dataSize());
        assert(state.func->data.atCell(state.itr[1]).isA<Symbol>());
        ASM_LOG("\tRETURNL " << state.itr[1] << ": " <<
                state.func->data.atCell(state.itr[1]) <<
                " stackSize: " << dataStack.size() <<
                " returnPos: " << state.returnPos);
        if(state.returnPos < dataStack.size())
        {
          dataStack[state.returnPos] = env->find(state.func->data.atCell(state.itr[1]));
        }
        else
        {
          assert(state.returnPos == dataStack.size());
          dataStack.push_back(env->find(state.func->data.atCell(state.itr[1])));
        }
        state.itr += 2;
        break;
        
      case INCRET:
        // increase return position by 1
        state.returnPos++;
        ASM_LOG("\tINCRET returnPos: " << state.returnPos);
        state.itr++;
        break;

      case FUNCALL:
        assert((state.itr + 2) < state.end);
        assert(state.itr[2] < state.func->data.size());
        assert(state.func->data.atCell(state.itr[2]).isA<Function>());
        // @todo check number of arguments
        ASM_LOG("\tFUNCALL nargs: " << state.itr[1] <<
                " func: " << state.itr[2] << ": " <<
                state.func->data.atCell(state.itr[2]) <<
                " stackSize: " << dataStack.size() <<
                " returnPos: " << (dataStack.size() - state.itr[1]));

        // @todo tail recurion optimization, don't emplace if
        //       it is the end of the function
        state.returnPos -= state.itr[1];
        executionStack.emplace_back(state, state.itr + 3);
        LOG_DATA_STACK(dataStack);
        state = ExecutionState(state.func->data.atCell(state.itr[2]).as<Function>(),
                               dataStack.size() - state.itr[1]);
        state.func->makeReference(dataStack.end() - state.func->numArguments(), alloc);
        break;
      
      case DEFINES:
        // define a symbol
        assert((state.itr + 1) < state.end);
        assert(state.itr[1] < state.func->dataSize());
        assert(state.func->data.atCell(state.itr[1]).isA<Symbol>());
        assert(dataStack.size() > 0);
        ASM_LOG("\tDEFINES " << state.itr[1] << ": " <<
                state.func->data.atCell(state.itr[1]) <<
                " stackSize: " << dataStack.size() <<
                " returnPos: " << state.returnPos);
        env->set(state.func->data.atCell(state.itr[1]), dataStack.back());
        state.itr += 2;
        break;
      
      default:
        // @todo proper exception
        ASM_LOG("unkown instruction " << *state.itr);
        throw 1;
      }
    } // while itr != end
    if(state.func->numArguments() > 1)
    {
      pop(state.func->numArguments()-1);
    }
    LOG_DATA_STACK(dataStack);
    if(executionStack.empty())
    {
      break;
    }
    else
    {
      ASM_LOG("----------------------------------");
      ASM_LOG("executionStack: " << executionStack.size() << " --> POP");
      state = executionStack.back();
      executionStack.pop_back();
      ASM_LOG("FUNC\t" << state.func <<
              " itr: " << (state.itr - state.func->cbegin()) << "/" << state.func->numInstructions() <<
              " nargs: " << state.func->numArguments() <<
              " returnPos: " << state.returnPos);
    }
  }
}



