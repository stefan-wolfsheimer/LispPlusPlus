#include <lpp/scheme/language.h>
#include <lpp/scheme/context.h>
#include <lpp/scheme/lambda_form.h>
#include <lpp/scheme/procedure_call_form.h>
#include <lpp/core/vm.h>
#include <lpp/core/cell.h>
#include <lpp/core/exception.h>
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
using IllFormed = Lisp::IllFormed;

using Any = Lisp::Any;
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
using FunctionForm = Lisp::TypeOf<Function>;
using AnyForm = Lisp::TypeOf<Any>;
using ConsOf = Lisp::ConsOf;
using ListOf = Lisp::ListOf;
using SymbolEq = Lisp::SymbolEq;

  
// scheme specific
using Language = Lisp::Scheme::Language;
using Context = Lisp::Scheme::Context;
using ProcedureCallForm = Lisp::Scheme::ProcedureCallForm;

Language::Language()
{
  expression = nullptr;
}

bool Language::match(const Cell & cell) const
{
  return topLevelForm->match(cell);
}

bool Language::isInstance(const Cell & cell) const
{
  return topLevelForm->isInstance(cell);
}

Object Language::compile(const Cell & cell) const
{
  Context ctx(getAllocator());
  //MainMatcher matcher;
  if(match(cell))
  {
  }
  else
  {
    throw IllFormed(cell);
  }
  return ctx.getFunctionObject();
}

void Language::init()
{
  auto allocator = getAllocator();
  Guard _lock(allocator);
  // @todo move IdempotentForm trait to Scheme submodule (it depends on the grammar of the language)
  // @todo define is actually not a expression
  //       <top level form> = <expression> | <define>
  //       <body form> = <define>* <expression>*
  //expressions 11.4
  expression = makeRoot<ChoiceOf>();
  expression->add(make<IdempotentForm>(Context::idempotentForm));
  expression->add(make<SymbolForm>(Context::symbolForm));
  expression->add(make<LambdaForm>(make<ConsOf>(expression,
                                                make<NilForm>())));
  expression->add(make<ProcedureCallForm>(expression,
                                          Context::procedureCallArgumentForm,
                                          Context::procedureCallForm));
  topLevelForm = makeRoot<ChoiceOf>(std::vector<Form*>{
      make<ConsOf>(make<SymbolEq>(allocator->make<Symbol>("define")),
                   make<ConsOf>(make<SymbolForm>(),
                                make<ConsOf>(expression,
                                             make<NilForm>()), Context::defineForm)),
      expression});
}
