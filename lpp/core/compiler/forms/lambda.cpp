#include <lpp/core/compiler/forms/lambda.h>
#include <lpp/core/util.h>
#include <lpp/core/exception.h>
#include <lpp/core/opcode.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/compiler/scope_guard.h>

#include <lpp/core/gc/garbage_collector.h>

#include <lpp/core/types/cons.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/forms/list_of.h>
#include <lpp/core/types/forms/type_of.h>

using Lambda = Lisp::Lambda;
using Nil = Lisp::Nil;
using Any = Lisp::Any;
using Symbol = Lisp::Symbol;
using ConsOf = Lisp::Form::ConsOf;
using ListOf = Lisp::Form::ListOf;
using SymbolForm = Lisp::Form::TypeOf<Symbol>;
using NilForm = Lisp::Form::TypeOf<Nil>;
using AnyForm = Lisp::Form::TypeOf<Any>;


Lambda::Lambda() : pattern(nullptr)
{
}

void Lambda::init()
{
  Lisp::GarbageCollector::Guard _lock(*getCollector());
  pattern = makeRoot<ConsOf>(make<SymbolForm>(),
                             make<ConsOf>(make<ListOf>(make<SymbolForm>()),
                                          make<ConsOf>(make<AnyForm>(),
                                                       make<NilForm>())));
}

bool Lambda::isInstance(const Cell & cell) const
{
  return pattern->isInstance(cell);
}

void Lambda::compile(Jit & jit, Function * f0, const Cell & obj) const
{
  // ((a b c) ...)
  //
  if(pattern->isInstance(obj))
  {
    Function * f = jit.gc->makeRoot<Function>();
    Object funcObject(f);
    Cons * cons = obj.as<Cons>()->getCdrCell().as<Cons>();
    ScopeGuard scope_guard(jit.scope, funcObject);
    Lisp::forEachCar(cons->getCarCell(),
                     [&jit, &obj, &f](const Cell & cell) {
                       if(jit.scope->isSet(cell))
                       {
                         throw ArgumentGivenTwice(cell, obj);
                       }
                       jit.scope->add(cell);
                     });
    // (lambda (...) body)
    //               |
    jit.compile(f, cons->getCdrCell().as<Cons>()->getCarCell());
    f->shrink();

    f0->appendInstruction(RETURNV, f0->dataSize());
    f0->appendData(funcObject);
  }
  else
  {
    throw IllFormed(obj);
  }
}
