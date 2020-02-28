#include <lpp/scheme/builder.h>
#include <lpp/scheme/language.h>
#include <lpp/scheme/lambda_form.h>
#include <lpp/scheme/procedure_call_arg_decorator.h>
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
#include <lpp/core/types/reference.h>
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
using Reference = Lisp::Reference;
using Nil = Lisp::Nil;


// gc
using Guard = Lisp::Allocator::Guard;

// scheme specific
using Language = Lisp::Scheme::Language;
using Builder = Lisp::Scheme::Builder;
using ProcedureCallArgDecorator = Lisp::Scheme::ProcedureCallArgDecorator;

// forms 
using ChoiceOfForm = Lisp::ChoiceOf<Builder>;
using IdempotentForm = Lisp::TypeOf<Idempotent, Builder>;
using ReferenceForm = Lisp::TypeOf<Reference, Builder>;
using ListOfForm = Lisp::ListOf<Builder>;
using SymbolForm = Lisp::TypeOf<Symbol, Builder>;
using NilForm = Lisp::TypeOf<Nil, void>;
using FunctionForm = Lisp::TypeOf<Function, void>;
using AnyForm = Lisp::TypeOf<Any, void>;
using ConsOfForm = Lisp::ConsOf<Builder>;
using SymbolEqForm = Lisp::SymbolEq<Builder>;


//////////////////////////////////////////////

Language::Language()
{
  expression = nullptr;
}

bool Language::isInstance(const Cell & cell) const
{
  return topLevelForm->isInstance(cell);
}

Object Language::compile(const Cell & cell) const
{
  Builder builder(getAllocator());
  if(topLevelForm->match(cell, builder))
  {
  }
  else
  {
    throw IllFormed(cell);
  }
  return builder.getFunctionObject();
}

Form * Language::symbolEq(const std::string & name)
{
  return make<SymbolEq<Builder>>(getAllocator()->make<Symbol>(name));
}

///////////////////////////////////////////////////////
ProcedureCallArgDecorator::ProcedureCallArgDecorator(Lisp::FormBuilder<Builder> * _expression)
{
  cells.push_back(Cell(_expression));
  expression = _expression;
}
  
bool ProcedureCallArgDecorator::isInstance(const Cell & cell) const
{
  return expression->isInstance(cell);
}

bool ProcedureCallArgDecorator::match(const Cell & cell, Builder & builder) const
{
  return expression->match(cell, builder);
}
//////////////////////////////////////////

void Language::init()
{
  auto allocator = getAllocator();
  Guard _lock(allocator);
  // @todo move IdempotentForm trait to Scheme submodule (it depends on the grammar of the language)
  // @todo define is actually not a expression
  //       <top level form> = <expression> | <define>
  //       <body form> = <define>* <expression>*
  //expressions 11.4
  //auto defineForm = ;

  expression = makeRoot<ChoiceOfForm>();
  expression->add(make<IdempotentForm>(&Builder::idempotent));
  expression->add(make<ReferenceForm>(&Builder::reference));
  expression->add(make<SymbolForm>(&Builder::symbol));

  expression->add(make<ConsOfForm>(symbolEq("lambda"),
                                   make<LambdaForm>(make<ListOfForm>(make<SymbolForm>(),
                                                                     &Builder::lambdaArgument),
                                                    make<ConsOfForm>(expression,
                                                                     make<NilForm>()))));

  //@todo parse user defined macros

  expression->add(make<ListOfForm>(make<ProcedureCallArgDecorator>(expression),
                                   nullptr,
                                   &Builder::funcall));

  // top level form
  topLevelForm = makeRoot<ChoiceOfForm>();
  topLevelForm->add(make<ConsOfForm>(symbolEq("define"),
                                     make<ConsOfForm>(make<SymbolForm>(),
                                                      make<ConsOfForm>(expression,
                                                                       make<NilForm>()),
                                                      &Builder::define)));
  topLevelForm->add(expression);
}
