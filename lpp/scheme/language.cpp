#include <lpp/scheme/builder.h>
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
using Context = Lisp::Scheme::Context;
using Language = Lisp::Scheme::Language;
using ProcedureCallForm = Lisp::Scheme::ProcedureCallForm;
using Builder = Lisp::Scheme::Builder;

// forms 
using ChoiceOfForm = Lisp::ChoiceOf<Builder>;
using IdempotentForm = Lisp::TypeOf<Idempotent, Builder>;
using ReferenceForm = Lisp::TypeOf<Reference, Builder>;
using SymbolForm = Lisp::TypeOf<Symbol, Builder>;
using NilForm = Lisp::TypeOf<Nil, void>;
using FunctionForm = Lisp::TypeOf<Function, void>;
using AnyForm = Lisp::TypeOf<Any, void>;
using ConsOfForm = Lisp::ConsOf<Builder>;
using ListOf = Lisp::ListOf;
using SymbolEqForm = Lisp::SymbolEq<Builder>;


// @todo move to separate module


void Builder::idempotent(const Cell & cell)
{
  Context::idempotentForm(nullptr, cell);
}

void Builder::reference(const Cell & cell)
{
  Context::referenceForm(nullptr, cell);
}

void Builder::symbol(const Cell & cell)
{
  Context::symbolForm(nullptr, cell);
}

void Builder::define(const Cell & car, const Cell & cdr)
{
  Context::defineForm(nullptr, car, nullptr, cdr);
}
//////////////////////////////////////////////

Language::Language()
{
  expression = nullptr;
}

bool Language::match(const Cell & cell) const
{
  Builder builder;
  return topLevelForm->match(cell, builder);
}

bool Language::isInstance(const Cell & cell) const
{
  return topLevelForm->isInstance(cell);
}

Object Language::compile(const Cell & cell) const
{
  Context ctx(getAllocator());
  Builder builder;
  // @todo define toplevel as FormBuilder<Builder>
  if(topLevelForm->match(cell, builder))
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
  expression = makeRoot<ChoiceOfForm>();
  expression->add(make<IdempotentForm>(&Builder::idempotent));
  expression->add(make<ReferenceForm>(&Builder::reference));
  expression->add(make<SymbolForm>(&Builder::symbol));
  expression->add(make<LambdaForm>(make<ConsOfForm>(expression,
                                                    make<NilForm>())));
  expression->add(make<ProcedureCallForm>(expression,
                                          Context::procedureCallArgumentForm,
                                          Context::procedureCallForm));
  topLevelForm = makeRoot<ChoiceOfForm>(
                                        std::vector<Form*>{
                                          make<ConsOfForm>(make<SymbolEqForm>(allocator->make<Symbol>("define")),
                                                           make<ConsOfForm>(make<SymbolForm>(),
                                                                            make<ConsOfForm>(expression,
                                                                                             make<NilForm>()),
                                                                            &Builder::define)),
                                            expression});
}
