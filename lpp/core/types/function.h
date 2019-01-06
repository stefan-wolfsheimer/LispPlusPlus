/******************************************************************************
Copyright (c) 2018, Stefan Wolfsheimer

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
#include <cstdint>
#include <vector>
#include <lpp/core/opcode.h>
#include <lpp/core/object.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/container.h>
#include <lpp/core/types/array.h>


namespace Lisp
{
  class Object;
  class Vm;
  
  class Function : public Container
  {
  public:
    friend class Vm;
    using Code = std::vector<InstructionType>;
    
    Function(const Code & instr, const Array & data);
    Function(Code && instr, Array && data);

    //////////////////////////////////////////////////
    // implementation of the Container interface
    //////////////////////////////////////////////////
    virtual void forEachChild(std::function<void(const Cell&)> func) const override
    {
      data.forEachChildImpl(func);
    }

    virtual TypeId getTypeId() const override
    {
      return data.getTypeIdImpl();
    }

    virtual bool greyChildren() override
    {
      return data.greyChildrenImpl();
    }

    virtual void resetGcPosition() override
    {
      return data.resetGcPositionImpl();
    }

    virtual bool recycleNextChild() override
    {
      return data.recycleNextChildImpl();
    }

  private:
    Code instructions;
    Array data;
  };
}

inline Lisp::Function::Function(const std::vector<InstructionType> & instr,
                                const Array & _data)
  : instructions(instr), data(_data)
{
}

inline Lisp::Function::Function(Code && instr, Array && _data)
  : instructions(std::move(instr)), data(std::move(_data))
{
}
