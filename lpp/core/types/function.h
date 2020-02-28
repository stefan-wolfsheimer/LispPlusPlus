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
#include <limits>
#include <lpp/core/opcode.h>
#include <lpp/core/object.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/types/container.h>
#include <lpp/core/types/array.h>
#include <lpp/core/types/reference.h>


namespace Lisp
{
  class Object;
  class Vm;
  class Allocator;

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
    friend class Vm; //@todo remove this friendship
    friend class Continuation;
    using Code = std::vector<InstructionType>;
    using const_iterator = Code::const_iterator;
    static const std::size_t notFound;

    Function();
    Function(const Code & instr, const Array & data);
    Function(Code && instr, Array && data);

    /**
     * Add instructions 
     */
    inline void addPUSHV(const Cell & rhs);
    inline void addRETURNS(InstructionType offset);
    inline void addRETURNL(const Cell & rhs);
    inline void addPUSHL(const Cell & rhs);
    inline void addFUNCALL(const InstructionType & n);
    inline void addDEFINES(const Cell & symbol);

    inline void appendData(const Cell & rhs);
    inline void addArgument(const Cell & cell);
    
    /**
     * Modify the ith argument: make it shareable
     */
    inline Object shareArgument(std::size_t i);

    /**
     * Number of static data elements.
     */
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

    /**
     * Return the position of the argument.
     * @param cell symbol to search
     * @return position of the argument or Function::notFound
     */
    inline std::size_t getArgumentPos(const Cell & cell) const;

    inline const_iterator cbegin() const;
    inline const_iterator cend() const;

    /**
     * Return a const reference of the ith data element.
     *
     * @param i position of the data element [0, dataSize())
     */
    inline const Cell & atCell(std::size_t i) const;

    /**
     * Return a const reference to the ith data element.
     * If the cell is a a reference, the reference is resolved
     * and the value is returned.
     */
    inline const Cell & getValue(std::size_t i) const;

    /**
     * Modifies stack values to references for each argument
     * that has reference trait
     */
    inline void makeReference(std::vector<Cell>::iterator stack_itr);

    void disassemble(std::ostream & ost) const;
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

inline Lisp::Object Lisp::Function::shareArgument(std::size_t i)
{
  assert(i < argumentTraits.size());
  if(argumentTraits[i].isReference())
  {
    return data[argumentTraits[i].getReferenceIndex()];
  }
  else
  {
    auto allocator = getAllocator();
    //@todo using guard
    //Guard _lock(allocator);
    //argumentTraits[i].setReference(data.size());
    //Cell obj(gc->make<Reference>(data.atCell(i), Lisp::nil))
    //appendData(obj);
    //return Object(obj);
    Object obj(allocator->makeRoot<Reference>(data.atCell(i), Lisp::nil));
    argumentTraits[i].setReference(data.size());
    appendData(obj);
    return obj;
  }
}

inline void Lisp::Function::addPUSHV(const Cell & rhs)
{
  instructions.push_back(PUSHV);
  instructions.push_back(data.size());
  data.append(rhs);
}

inline void Lisp::Function::addRETURNS(InstructionType offset)
{
  instructions.push_back(RETURNS);
  instructions.push_back(offset);
}

inline void Lisp::Function::addRETURNL(const Cell & rhs)
{
  assert(rhs.isA<Symbol>());
  instructions.push_back(RETURNL);
  instructions.push_back(data.size());
  data.append(rhs);
}

inline void Lisp::Function::addPUSHL(const Cell & rhs)
{
  assert(rhs.isA<Symbol>());
  instructions.push_back(PUSHL);
  instructions.push_back(data.size());
  data.append(rhs);
}

inline void Lisp::Function::addFUNCALL(const InstructionType & n)
{
  instructions.push_back(FUNCALL);
  instructions.push_back(n);
}

inline void Lisp::Function::addDEFINES(const Cell & symbol)
{
  assert(symbol.isA<Symbol>());
  instructions.push_back(DEFINES);
  instructions.push_back(data.size());
  data.append(symbol);
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

inline std::size_t Lisp::Function::getArgumentPos(const Cell & cell) const
{
  std::size_t n = argumentTraits.size();
  for(std::size_t i = 0; i < n; i++)
  {
    assert(data[i].as<Symbol>());
    if(data[i].as<Symbol>() == cell.as<Symbol>())
    {
      return i;
    }
  }
  return Lisp::Function::notFound;
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

inline const Lisp::Cell & Lisp::Function::getValue(std::size_t i) const
{
  const Cell & ret(data.atCell(i));
  if(ret.isA<Reference>())
  {
    return ret.as<Reference>()->getValue();
  }
  else
  {
    return ret;
  }
}

inline void Lisp::Function::makeReference(std::vector<Cell>::iterator stack_itr)
{
  //@todo only execute, if function has at least one reference
  std::size_t i = 0;
  stack_itr-= numArguments();
  for(const ArgumentTraits & traits : argumentTraits)
  {
    if(traits.isReference())
    {
      if(!stack_itr->isA<Reference>())
      {
        data[traits.getReferenceIndex()].as<Reference>()->setCdr(*stack_itr);
        *stack_itr = data[traits.getReferenceIndex()];
      }
    }
    ++stack_itr;
    i++;
  }
}
