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
#include "lisp_vm.h"
#include "types/lisp_function.h"
#include "lisp_config.h"
#include "lisp_opcode.h"

#ifdef NDEBUG
const bool Lisp::Vm::withDebug = false;
#else
const bool Lisp::Vm::withDebug = true;
#endif

Lisp::Vm::Vm(std::shared_ptr<GarbageCollector> _consFactory)
  : consFactory( _consFactory ?
                 _consFactory :
                 std::make_shared<GarbageCollector>())
{
  dataStack.reserve(1024);
  values.reserve(1024);
  values.push_back(Lisp::nil);
}

void Lisp::Vm::eval(const Function * func)
{
  Function::ProgramType::const_iterator itr = func->instr.begin();
  while(itr != func->instr.end())
  {
    switch(itr->first)
    {
    case Lisp::SETV:
      assert(itr->second < func->data.size());
      values.resize(1);
      values[0] = func->data[itr->second];
      break;
    default:
      //@todo error handling
      break;
    }
    ++itr;
  }
}



