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
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/container.h>
#include <lpp/core/types/array.h>
#include <lpp/core/types/reference.h>


namespace Lisp
{
  class Object;
  class Vm;

  class ArgumentTraits
  {
  public:
    ArgumentTraits();
    inline bool isReference() const;
    inline void setReference(std::size_t index);
    inline std::size_t getReferenceIndex() const;
  private:
    bool ref;
    std::size_t refIndex;
  };

  class Function : public Container
  {
  public:
    friend class Vm;
    using Code = std::vector<InstructionType>;
    using const_iterator = Code::const_iterator;

    Function();
    Function(const Code & instr, const Array & data);
    Function(Code && instr, Array && data);

    inline void appendInstruction(const InstructionType & i1);
    inline void appendInstruction(const InstructionType & i1,
                                  const InstructionType & i2);
    inline void appendInstruction(const InstructionType & i1,
                                  const InstructionType & i2,
                                  const InstructionType & i3);
    inline void appendData(const Cell & rhs);
    inline void addArgument(const Cell & cell);
    inline Object shareArgument(std::size_t i, std::shared_ptr<GarbageCollector> gc);

    inline std::size_t dataSize() const;
    inline std::size_t numArguments() const;

    /**
     * Number of instructions
     */
    inline std::size_t numInstructions() const;

    /**
     * Shrink all internal vectors to fit size.
     */
    inline void shrink();

    /**
     * Get properties of ith function argument.
     * @param index of argument i in [0 ... numArguments() )
     */
    inline const ArgumentTraits & getArgumentTraits(const std::size_t i) const;
    inline ArgumentTraits & getArgumentTraits(const std::size_t i);

    inline const_iterator cbegin() const;
    inline const_iterator cend() const;

    /**
     * return a const reference of the ith data element
     */
    inline const Cell & atCell(std::size_t i) const;

    //////////////////////////////////////////////////
    // implementation of the Container interface
    //////////////////////////////////////////////////
    virtual void forEachChild(std::function<void(const Cell&)> func) const override
    {
      data.forEachChildImpl(func);
    }

    virtual TypeId getTypeId() const override
    {
      return TypeTraits<Function>::getTypeId();
    }

    virtual bool greyChildren() override
    {
      return data.greyChildrenImpl();
    }

    virtual void resetGcPosition() override
    {
      data.resetGcPositionImpl();
    }

    virtual bool recycleNextChild() override
    {
      return data.recycleNextChildImpl();
    }

  private:
    std::vector<ArgumentTraits> argumentTraits;
    Code instructions;
    Array data;
  };
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
///////////////////////////////////////////////////////////////////////////////
inline Lisp::ArgumentTraits::ArgumentTraits() : ref(false), refIndex(0)
{
}

inline bool Lisp::ArgumentTraits::isReference() const
{
  return ref;
}

inline void Lisp::ArgumentTraits::setReference(std::size_t index)
{
  refIndex = index;
  ref = true;
}

inline std::size_t Lisp::ArgumentTraits::getReferenceIndex() const
{
  return refIndex;
}


///////////////////////////////////////////////////////////////////////////////
inline Lisp::Function::Function()
{
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

inline void Lisp::Function::appendData(const Cell & rhs)
{
  data.append(rhs);
}

inline void Lisp::Function::addArgument(const Cell & cell)
{
  argumentTraits.push_back(ArgumentTraits());
  appendData(cell);
}

inline Lisp::Object Lisp::Function::shareArgument(std::size_t i, std::shared_ptr<GarbageCollector> gc)
{
  assert(i < argumentTraits.size());
  if(argumentTraits[i].isReference())
  {
    return data[argumentTraits[i].getReferenceIndex()];
  }
  else
  {
    Object obj(gc->makeRoot<Reference>(data.atCell(i), Lisp::nil));
    argumentTraits[i].setReference(data.size());
    appendData(obj);
    return obj;
  }
}

inline void Lisp::Function::appendInstruction(const InstructionType  & i1)
{
  instructions.push_back(i1);
}

inline void Lisp::Function::appendInstruction(const InstructionType  & i1, const InstructionType  & i2)
{
  instructions.push_back(i1);
  instructions.push_back(i2);
}

inline void Lisp::Function::appendInstruction(const InstructionType  & i1,
                                              const InstructionType  & i2,
                                              const InstructionType  & i3)
{
  instructions.push_back(i1);
  instructions.push_back(i2);
  instructions.push_back(i3);
}

inline std::size_t Lisp::Function::dataSize() const
{
  return data.size();
}

inline std::size_t Lisp::Function::numArguments() const
{
  return argumentTraits.size();
}

inline std::size_t Lisp::Function::numInstructions() const
{
  return instructions.size();
}

inline void Lisp::Function::shrink()
{
  data.shrink();
  instructions.shrink_to_fit();
  argumentTraits.shrink_to_fit();
}

inline const Lisp::ArgumentTraits & Lisp::Function::getArgumentTraits(const std::size_t i) const
{
  return argumentTraits[i];
}

inline Lisp::ArgumentTraits & Lisp::Function::getArgumentTraits(const std::size_t i)
{
  return argumentTraits[i];
}


inline Lisp::Function::const_iterator Lisp::Function::cbegin() const
{
  return instructions.cbegin();
}

inline Lisp::Function::const_iterator Lisp::Function::cend() const
{
  return instructions.cend();
}

inline const Lisp::Cell & Lisp::Function::atCell(std::size_t i) const
{
  return data.atCell(i);
}
