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
                                            std::size_t _returnPos,
                                            std::size_t _stackPos)
  : f(_f), stackPos(_stackPos), returnPos(_returnPos)
{
  itr = f->cbegin();
  end = f->cend();
}

Continuation::Continuation(const Cell & func,
                           const std::shared_ptr<Env> & _env)
  : callStack({ContinuationState(func.as<Function>(), 0, 0)}),
    env(_env)
{
  stack.push_back(func);
  dsPosition = 0;
}

TypeId Continuation::getTypeId() const
{
  return TypeTraits<Continuation>::getTypeId();
}

void Continuation::forEachChild(std::function<void(const Cell&)> func) const
{
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
  dsPosition = 0;
}

bool Continuation::recycleNextChild()
{
  if(dsPosition < stack.size())
  {
    if(!stack[dsPosition].isA<Collectible>())
    {
      stack[dsPosition] = Lisp::nil;
    }
    return ++dsPosition == stack.size();
  }
  return true;
}

Cell & Continuation::eval()
{
  ContinuationState s(callStack.back());
  while(!callStack.empty())
  {
    s = callStack.back();
    ASM_LOG("----------------------------------");
    ASM_LOG("eval      " << s.f);
    ASM_LOG("returnPos " << s.returnPos);
    ASM_LOG("stackPos  " << s.stackPos);
    ASM_LOG("pos       " << (s.itr - s.f->cbegin()));
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
                " stackPos: " << s.stackPos);
        assert(s.stackPos < stack.size());
        stack[s.stackPos] = s.f->getValue(s.itr[1]);
        if(s.stackPos < stack.size())
        {
          stack[s.stackPos] = s.f->getValue(s.itr[1]);
        }
        else
        {
          assert(s.stackPos == stack.size());
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
                " stackPos: " << s.stackPos);
        assert(s.stackPos < stack.size());
        if(stack.size() - s.itr[1] != s.stackPos)
        {
          stack[s.stackPos] = stack[stack.size() - s.itr[1]];
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
                " stackPos: " << s.stackPos);
        assert(s.stackPos < stack.size());
        stack[s.stackPos] = env->find(s.f->data.atCell(s.itr[1]));
        s.itr += 2;
        break;

      case INCRET:
        // increase return position by 1
        assert(s.stackPos < stack.size());
        stack.push_back(Lisp::nil);
        s.stackPos++;
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " INCRET stackPos: " << s.stackPos);
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
                " stackPos: " << s.stackPos);
        env->set(s.f->data.atCell(s.itr[1]), Object(stack.back()));
        s.itr += 2;
        break;

      case FUNCALL:
        LOG_DATA_STACK(stack);
        assert((s.itr + 1) < s.end);
        if(!(stack.size() >= (s.itr[1] + 1)))
        {
          std::cout << "stack.size():" << stack.size() << "s.itr[1]:" << s.itr[1] << std::endl;
        }
        assert(stack.size() >= (s.itr[1] + 1));
        assert(stack[stack.size() - s.itr[1] - 1].isA<Function>());
        // @todo check number of arguments
        ASM_LOG("\t" << (s.itr - s.f->cbegin()) <<
                " FUNCALL nargs: " << s.itr[1] <<
                " func: " << stack[stack.size() - s.itr[1] - 1].as<Function>() <<
                " stackPos: " << (stack.size() - s.itr[1] - 1) <<
                " nextitr: " << s.f << ":" <<
                ((s.itr + 2) - s.f->cbegin()) << "/" <<
                (s.end - s.f->cbegin()));
        if(s.itr + 2 == s.end)
        {
          // tail recursion
          // @todo check num args
          s.stackPos = stack.size() - s.itr[1] - 1;
          s.f = stack[s.stackPos].as<Function>();
          s.itr = s.f->cbegin();
          s.end = s.f->cend();
        }
        else
        {
          std::size_t p = stack.size() - s.itr[1] - 1;
          s.itr++;
          s.itr++;
          callStack.back() = s;
          ASM_LOG("stackPos:" << s.stackPos);
          callStack.emplace_back(stack[p].as<Function>(), p, p);
          s = callStack.back();
          ASM_LOG("\t" << (s.itr - s.f->cbegin()) <<
                  " BEGINFUNC nargs: " << s.f->numArguments() <<
                  " Function " << s.f);
        }
        break;

      default:
        // @todo proper exception
        ASM_LOG("unkown instruction " << *s.itr);
        throw 1;
      }
    } // while s.itr != s.end
    LOG_DATA_STACK(stack);
    stack.erase(stack.begin() + s.stackPos + 1,
                stack.end());
    LOG_DATA_STACK(stack);
    callStack.pop_back();
  }

  //while(s.stackPos + 1 > stack.size())
  //{
  //throw 1;
  //stack.push_back(Lisp::nil);
  //}
  //for(auto itr = stack.begin() + s.stackPos + 1; itr < stack.end(); ++itr)
  //{
  //*itr = Lisp::nil;
  //}
  stack.erase(stack.begin() + s.stackPos + 1,
              stack.end());
  return stack.back();
}
