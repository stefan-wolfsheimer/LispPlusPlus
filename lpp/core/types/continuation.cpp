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
#define LOG_DATA_STACK(STACK)  {                            \
    ASM_LOG("STACK_SIZE: " << STACK.size());                \
    for(std::size_t i = 0; i < STACK.size(); i++)           \
    {                                                       \
      ASM_LOG("STACK #" << i << "=<" << STACK[i] << ">");   \
    }                                                       \
  }
#else
#define LOG_DATA_STACK(STACK)
#endif


inline ContinuationState::ContinuationState(Function * _f,
                                            std::size_t _stackFramePos)
  : f(_f), stackFramePos(_stackFramePos)
{
  itr = f->cbegin();
  end = f->cend();
}

Continuation::Continuation(const Cell & func,
                           const std::shared_ptr<Env> & _env)
  : callStack({ContinuationState(func.as<Function>(), 0)}),
    env(_env)
{
  assert(func.isA<Function>());
  assert(stack.size() == func.as<Function>()->numArguments());
  stack.push_back(func);
  dsPosition = 0;
}

Continuation::Continuation(std::vector<Lisp::Cell> && _stack, const std::shared_ptr<Env> & _env)
  : stack(std::move(_stack)), env(_env)
{
  assert(!stack.empty());
  assert(stack.front().isA<Function>());
  assert((stack.front().as<Function>()->numArguments() + 1) == stack.size());
  callStack.emplace_back(stack.front().as<Function>(), 0);
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
    std::size_t sf = s.stackFramePos;
    ASM_LOG("----------------------------------");
    ASM_LOG("eval        " << s.f);
    ASM_LOG("nargs       " << s.f->numArguments());
    ASM_LOG("stackFrame  " << sf);
    ASM_LOG("pos         " << (s.itr - s.f->cbegin()) << "/"  << (s.end - s.f->cbegin()));
    LOG_DATA_STACK(stack);
    ASM_LOG("----------------------------------");
    s.f->makeReference(stack.end());
    while(s.itr != s.end)
    {
      switch(*s.itr)
      {
      case PUSHV:
        assert((s.itr + 1) < s.end);
        assert(s.itr[1] < s.f->dataSize());
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) <<
                " PUSHV @" << s.itr[1] << "=<" <<
                s.f->data.atCell(s.itr[1]) << ">" <<
                " --> #" << stack.size());
        stack.push_back(s.f->getValue(s.itr[1]));
        s.itr += 2;
        break;

      case PUSHL:
        assert((s.itr + 1) < s.end);
        assert(s.itr[1] < s.f->dataSize());
        assert(s.f->data.atCell(s.itr[1]).isA<Symbol>());
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " RETURNL @" << s.itr[1] <<
                "=<" << s.f->data.atCell(s.itr[1]) << ">" <<
                " --> #" << stack.size() <<
                " stackSize: " << stack.size());
        stack.push_back(std::move(env->find(s.f->data.atCell(s.itr[1]))));
        s.itr += 2;
        break;

      case RETURNS:
        assert((s.itr + 1) < s.end);
        assert(s.itr[1] <= stack.size());
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " RETURNS " << s.itr[1] << " func: " <<
                s.f << " #" << ((stack.size() - s.itr[1])) << "=<" <<
                *(stack.end() - s.itr[1]) << ">" <<
                " stackFrame: " << sf <<
                " / " << stack.size());
        assert(sf < stack.size());
        stack[sf] = *(stack.end() - s.itr[1]);
        stack.erase(stack.begin() + sf + 1, stack.end());
        s.itr += 2;
        break;

      case RETURNL:
        // return a value from function data
        // @todo exception if unbound
        assert((s.itr + 1) < s.end);
        assert(s.itr[1] < s.f->dataSize());
        assert(s.f->data.atCell(s.itr[1]).isA<Symbol>());
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) << " RETURNL @" << s.itr[1] <<
                "=<" << s.f->data.atCell(s.itr[1]) << ">" <<
                " --> #" << stack.size());
        stack.emplace_back(env->find(s.f->data.atCell(s.itr[1])));
        //assert(returnPos < stack.size());
        //stack[returnPos] = env->find(s.f->data.atCell(s.itr[1]));
        //stack[s.stackPos] = env->find(s.f->data.atCell(s.itr[1]));
        s.itr += 2;
        break;

      case DEFINES:
        // define a symbol
        assert((s.itr + 1) < s.end);
        assert(s.itr[1] < s.f->dataSize());
        assert(s.f->data.atCell(s.itr[1]).isA<Symbol>());
        assert(stack.size() > 0);
        ASM_LOG("\t"  << (s.itr - s.f->cbegin()) <<
                " DEFINES @" << s.itr[1] << "=<" <<
                s.f->data.atCell(s.itr[1]) << "> <-- " <<
                " #" << (stack.size() - 1) << "=<" <<
                stack.back() << ">");
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
                " func: " << stack[stack.size() - s.itr[1] - 1].as<Function>() <<
                " stackFrame: " << (stack.size() - s.itr[1] - 1) <<
                " nextitr: " << s.f << ":" <<
                ((s.itr + 2) - s.f->cbegin()) << "/" <<
                (s.end - s.f->cbegin()));
        if(s.itr + 2 == s.end)
        {
          // tail recursion
          // @todo check num args
          sf = s.stackFramePos;
          ASM_LOG("\tTAIL RECURSION stackFrame:" << sf);
          s.f = stack[stack.size() - s.itr[1] - 1].as<Function>();
          s.itr = s.f->cbegin();
          s.end = s.f->cend();
          s.f->makeReference(stack.end());
        }
        else
        {
          sf = stack.size() - s.itr[1] - 1;
          s.itr++;
          s.itr++;
          callStack.back() = s;
          callStack.emplace_back(stack[sf].as<Function>(), sf);
          s = callStack.back();
          ASM_LOG("\t" << (s.itr - s.f->cbegin()) <<
                  " BEGINFUNC nargs: " << s.f->numArguments() <<
                  " func: " << s.f <<
                  " stackFrame: " << sf);
          //assert(s.itr[1] + 2 <= stack.size());
          s.f->makeReference(stack.end());
        }
        break;

      default:
        // @todo proper exception
        ASM_LOG("unkown instruction " << *s.itr);
        throw 1;
      }
    } // while s.itr != s.end
    ASM_LOG("----------------------------------");
    ASM_LOG("return from " << s.f <<
            " pos:" << (s.itr - s.f->cbegin()) << "/"  << (s.end - s.f->cbegin()) <<
            " stackFrame:" << sf << "/" << stack.size());
    if(stack.size() > sf + 1)
    {
      LOG_DATA_STACK(stack);
      ASM_LOG("reduce    " << stack.size() << " -> " << (sf + 1));
      stack[sf] = std::move(stack.back());
      stack.erase(stack.begin() + sf + 1, stack.end());
    }
    else
    {
      //@todo implement reset function for Cell / Object
      //      and use stack.back().reset();
      assert((sf + 1) == stack.size());
    }
    callStack.pop_back();
  }
  return stack.back();
}
