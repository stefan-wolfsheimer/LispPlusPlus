#include <lpp/scheme/language.h>
#include <lpp/core/cell.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/forms/choice_of.h>
#include <lpp/core/types/forms/type_of.h>
#include <lpp/core/types/function.h>
#include <lpp/core/opcode.h>

// lisp core
using GarbageCollector = Lisp::GarbageCollector;
using Cell = Lisp::Cell;
using Object = Lisp::Object;
using InstructionType = Lisp::InstructionType;
using Function = Lisp::Function;
using Form = Lisp::Form;
using Idempotent = Lisp::Idempotent;


// gc
using Guard = Lisp::GarbageCollector::Guard;

// forms 
using ChoiceOf = Lisp::ChoiceOf;
using IdempotentForm = Lisp::TypeOf<Idempotent>;

// scheme specific
using Language = Lisp::Scheme::Language;

namespace Lisp
{
  namespace Scheme
  {
    namespace Details
    {
      struct Context
      {
        Context(GarbageCollector * _gc);
        ~Context();
        static void idempotentForm(const Form * form, const Cell & cell);
        static inline std::vector<Context*>& getContextStack();
        inline void finalize();
        GarbageCollector * gc ;
        Object funcObject;
        Function * f;
      };
    }
  }
}

using Context = Lisp::Scheme::Details::Context;

inline void Context::idempotentForm(const Form * form, const Cell & cell)
{
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->f;
    f->appendInstruction(Lisp::RETURNV, f->dataSize());
    f->appendData(cell);
  }
}

inline std::vector<Context*>& Context::getContextStack()
{
  static std::vector<Context*> ctxStack;
  return ctxStack;
}

inline Context::Context(GarbageCollector * _gc)
  : gc(_gc), funcObject(_gc->makeRoot<Function>())
{
  f = funcObject.as<Function>();
  getContextStack().push_back(this);
}

inline Context::~Context()
{
  getContextStack().pop_back();
}

inline void Context::finalize()
{
  f->shrink();
}


Language::Language()
{
  expression = nullptr;
}

bool Language::isInstance(const Cell & cell) const
{
  return expression->isInstance(cell);
}


Object Language::compile(const Cell & cell) const
{
  Details::Context ctx(getCollector());
  //MainMatcher matcher;
  if(isInstance(cell))
  {
  }
  else
  {
    //@todo throw execption
  }
  return ctx.funcObject;
}

void Language::init()
{
  Guard _lock(getCollector());
  auto collector = getCollector();
  // @todo move IdempotentForm trait to Scheme submodule (it depends on the grammar of the language)
  //expressions 11.4
  expression = makeRoot<ChoiceOf>(std::vector<Form*>{
      make<IdempotentForm>(Context::idempotentForm)
  });
}
