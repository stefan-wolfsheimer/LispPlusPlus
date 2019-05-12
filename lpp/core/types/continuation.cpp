#include <lpp/core/types/continuation.h>
#include <lpp/core/types/function.h>
#include <lpp/core/object.h>
#include <lpp/core/env.h>

using Cell = Lisp::Cell;
using Continuation = Lisp::Continuation;
using ContinuationState = Lisp::ContinuationState;
using TypeId = Lisp::TypeId;

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


inline ContinuationState::ContinuationState(Function * _f,
                                            std::size_t _returnPos)
  : f(_f), returnPos(_returnPos)
{
  itr = f->cbegin();
  end = f->cend();
}

/*
Continuation::Continuation(Function * f, const std::shared_ptr<Env> & _env)
  : callStack({ContinuationState(f, 0)}),
    csPosition(0),
    dsPosition(0),
    env(_env)
{
  f->getContainer()->grey(f);
  } */

Continuation::Continuation(const Cell & func,
                           const std::shared_ptr<Env> & _env)
  : callStack({ContinuationState(func.as<Function>(), 0)}),
    env(_env)
{
  stack.push_back(func);
  csPosition = 0;
  dsPosition = 0;
}

Continuation::~Continuation()
{
  stack.clear();
  callStack.clear();
}

TypeId Continuation::getTypeId() const
{
  return TypeTraits<Continuation>::getTypeId();
}

void Continuation::forEachChild(std::function<void(const Cell&)> func) const
{
  //for(const auto & c : callStack)
  //{
  //  func(Cell(c.getFunction(),
  //            TypeTraits<Function>::getTypeId()));
  //}
  for(const Cell & c : stack)
  {
    func(c);
  }
}

bool Continuation::greyChildren()
{
  if(dsPosition < stack.size())
  {
    stack[dsPosition].grey();
    if(++dsPosition == stack.size())
    {
      dsPosition = 0;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}

void Continuation::resetGcPosition()
{
  csPosition = 0;
  dsPosition = 0;
}

bool Continuation::recycleNextChild()
{
  bool ret = true;
  if(dsPosition < stack.size())
  {
    if(!stack[dsPosition].isA<Collectible>())
    {
      stack[dsPosition] = Lisp::nil;
    }
    if(++dsPosition < stack.size())
    {
      ret = false;
    }
  }
  return ret;
}

Cell & Continuation::eval()
{
  ContinuationState s(callStack.back());
  ASM_LOG("----------------------------------");
  ASM_LOG("eval " << s.f);
  ASM_LOG("returnPos " << s.returnPos);
  ASM_LOG("----------------------------------");
  LOG_DATA_STACK(stack);
  while(s.itr != s.end)
  {
    switch(*s.itr)
    {
    case RETURNV:
      // return a value from function data
      assert((s.itr + 1) < s.end);
      assert(s.itr[1] < s.f->dataSize());
      ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " RETURNV " << s.itr[1] << ": " <<
              s.f->data.atCell(s.itr[1]) <<
              " stackSize: " << stack.size() <<
              " returnPos: " << s.returnPos);
      assert(s.returnPos < stack.size());
      stack[s.returnPos] = s.f->getValue(s.itr[1]);
      if(s.returnPos < stack.size())
      {
        stack[s.returnPos] = s.f->getValue(s.itr[1]);
      }
      else
      {
        assert(s.returnPos == stack.size());
        stack.push_back(s.f->getValue(s.itr[1]));
      }
      s.itr += 2;
      break;

    case RETURNS:
      // return a value from stack
      assert((s.itr + 1) < s.end);
      assert(s.itr[1] <= stack.size());
      ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " RETURNS " << s.itr[1] << ": " <<
              *(stack.end() - s.itr[1]) <<
              " stackSize: " << stack.size() <<
              " returnPos: " << s.returnPos);
      assert(s.returnPos < stack.size());
      if(stack.size() - s.itr[1] != s.returnPos)
      {
        stack[s.returnPos] = stack[stack.size() - s.itr[1]];
      }
      s.itr += 2;
      break;

    case RETURNL:
      // return a value from function data
      // @todo exception if unbound
      assert((s.itr + 1) < s.end);
      assert(s.itr[1] < s.f->dataSize());
      assert(s.f->data.atCell(s.itr[1]).isA<Symbol>());
      ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " RETURNL " << s.itr[1]
              << ": " << s.f->data.atCell(s.itr[1]) <<
              " stackSize: " << stack.size() <<
              " returnPos: " << s.returnPos);
      assert(s.returnPos < stack.size());
      stack[s.returnPos] = env->find(s.f->data.atCell(s.itr[1]));
      s.itr += 2;
      break;

    case INCRET:
      // increase return position by 1
      assert(s.returnPos < stack.size());
      stack.push_back(Lisp::nil);
      s.returnPos++;
      ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " INCRET returnPos: " << s.returnPos);
      s.itr++;
      break;

    case DEFINES:
        // define a symbol
        assert((s.itr + 1) < s.end);
        assert(s.itr[1] < s.f->dataSize());
        assert(s.f->data.atCell(s.itr[1]).isA<Symbol>());
        assert(stack.size() > 0);
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) <<
                " DEFINES " << s.itr[1] << ": " <<
                s.f->data.atCell(s.itr[1]) <<
                " stackSize: " << stack.size() <<
                " returnPos: " << s.returnPos);
        env->set(s.f->data.atCell(s.itr[1]), Object(stack.back()));
        s.itr += 2;
        break;

    case FUNCALL:
      LOG_DATA_STACK(stack);
      assert((s.itr + 1) < s.end);
      assert(stack.size() >= (s.itr[1] + 1));
      assert(stack[stack.size() - s.itr[1] - 1].isA<Function>());
      // @todo check number of arguments
      ASM_LOG("\t" << (s.itr - s.f->cbegin()) <<
              " FUNCALL nargs: " << s.itr[1] <<
              " func: " << stack[stack.size() - s.itr[1] - 1].isA<Function>() <<
              " returnPos: " << (stack.size() - s.itr[1] - 1) <<
              " nextitr: " << s.f << ":" <<
              ((s.itr + 2) - s.f->cbegin()) << "/" <<
              (s.end - s.f->cbegin()));
      if(s.itr + 2 == s.end)
      {
        // tail recursion
        // @todo check num args
        s.returnPos = stack.size() - s.itr[1] - 1;
        s.f = stack[s.returnPos].as<Function>();
        s.itr = s.f->cbegin();
        s.end = s.f->cend();
      }
      else
      {
        // @todo continue
        throw 1;
      }
      // @todo tail recurion optimization, don't emplace if
      //       it is the end of the function
#if 0
      s.returnPos -= s.itr[1];
      executionStack.emplace_back(state, state.itr + 2);
        state = ExecutionState(dataStack[dataStack.size() - state.itr[1]].as<Function>(),
                               dataStack.size() - state.itr[1]);
        state.func->makeReference(dataStack.end() - state.func->numArguments());
#endif
        break;

    default:
      // @todo proper exception
      ASM_LOG("unkown instruction " << *s.itr);
      throw 1;
    }
  }
  while(s.returnPos + 1 > stack.size())
  {
    stack.push_back(Lisp::nil);
  }
  for(auto itr = stack.begin() + s.returnPos + 1; itr < stack.end(); ++itr)
  {
    *itr = Lisp::nil;
  }
  stack.erase(stack.begin() + s.returnPos + 1,
              stack.end());
  return stack.back();
}
