#include <lpp/scheme/language.h>
#include <lpp/scheme/context.h>
#include <lpp/scheme/lambda_form.h>
#include <lpp/core/vm.h>
#include <lpp/core/cell.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/types/forms/choice_of.h>
#include <lpp/core/types/forms/type_of.h>
#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/forms/list_of.h>
#include <lpp/core/types/forms/symbol_eq.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/opcode.h>

// lisp core
using Vm = Lisp::Vm;
using Allocator = Lisp::Allocator;
using Cell = Lisp::Cell;
using Object = Lisp::Object;
using InstructionType = Lisp::InstructionType;


using Form = Lisp::Form;
using Idempotent = Lisp::Idempotent;
using Function = Lisp::Function;
using Symbol = Lisp::Symbol;
using Nil = Lisp::Nil;

// gc
using Guard = Lisp::Allocator::Guard;

// forms 
using ChoiceOf = Lisp::ChoiceOf;
using IdempotentForm = Lisp::TypeOf<Idempotent>;
using SymbolForm = Lisp::TypeOf<Symbol>;
using NilForm = Lisp::TypeOf<Nil>;
using ConsOf = Lisp::ConsOf;
using ListOf = Lisp::ListOf;
using SymbolEq = Lisp::SymbolEq;

  
// scheme specific
using Language = Lisp::Scheme::Language;
using Context = Lisp::Scheme::Context;

namespace Lisp
{
  namespace Scheme
  {
    #if 0
    class LambdaForm : public ::Lisp::Form
    {
    public:
      LambdaForm(Form * _lambdaSymbol, Form * _argList, Form * _body)
      {
        cells.emplace_back(_lambdaSymbol);
        cells.emplace_back(_argList);
        cells.emplace_back(_body);
        lambdaSymbol = _lambdaSymbol;
        argList = _argList;
        body = _body;
      }

      bool isInstance(const Cell & cell) const override
      {      
        bool ret = false;
        if(cell.isA<Cons>() && lambdaSymbol->isInstance(cell.as<Cons>()->getCarCell()))
        {
          Function * f0 = Context::getContextStack().back()->getFunction();
          {
            Context ctx(getAllocator());
            ret = body->isInstance(cell.as<Cons>()->getCdrCell().as<Cons>()->getCdrCell());
            ctx.finalize();
            f0->appendInstruction(RETURNV, f0->dataSize());
            f0->appendData(ctx.getFunctionObject());
          }
        }
        return ret;
      }
    private:
      Form * lambdaSymbol;
      Form * argList;
      Form * body;
    };
#endif
  }
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
  Context ctx(getAllocator());
  //MainMatcher matcher;
  if(isInstance(cell))
  {
  }
  else
  {
    //@todo throw execption
  }
  return ctx.getFunctionObject();
}


//@todo move to vm: vm.compileAndEval(language, cell) (language interface with compile function)
Object Language::compileAndEval(Vm & vm, const Cell & cell) const
{
  Object func = compile(cell);
  assert(func.isA<Function>());
  return vm.evalAndReturn(func.as<Function>());
}

void Language::init()
{
  auto allocator = getAllocator();
  Guard _lock(allocator);
  // @todo move IdempotentForm trait to Scheme submodule (it depends on the grammar of the language)
  //expressions 11.4
  expression = makeRoot<ChoiceOf>(std::vector<Form*>{
      make<IdempotentForm>(Context::idempotentForm),
      make<SymbolForm>(Context::symbolForm),
      make<ConsOf>(make<SymbolEq>(allocator->make<Symbol>("define")),
                   make<ConsOf>(make<SymbolForm>(),
                                make<ConsOf>(make<IdempotentForm>(Context::idempotentForm),
                                             make<NilForm>()), Context::defineForm)),
      make<LambdaForm>(make<ConsOf>(make<IdempotentForm>(Context::idempotentForm),
                                    make<NilForm>()))});
}
