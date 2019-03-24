#include <lpp/core/compiler/forms/define.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/forms/list_of.h>
#include <lpp/core/types/forms/type_of.h>

using Define = Lisp::Define;
using Nil = Lisp::Nil;
using Any = Lisp::Any;
using Symbol = Lisp::Symbol;
using ConsOf = Lisp::Form::ConsOf;
using ListOf = Lisp::Form::ListOf;
using SymbolForm = Lisp::Form::TypeOf<Symbol>;
using NilForm = Lisp::Form::TypeOf<Nil>;
using AnyForm = Lisp::Form::TypeOf<Any>;


Define::Define(std::shared_ptr<GarbageCollector> gc)
{
  Lisp::GarbageCollector::Guard _lock(*gc);
  pattern = gc->make<ConsOf>(gc->make<SymbolForm>(),
                             gc->make<ConsOf>(gc->make<SymbolForm>(),
                                              gc->make<ConsOf>(gc->make<AnyForm>(),
                                                               gc->make<NilForm>())));
  cells.push_back(pattern);
}

bool Define::isInstance(const Cell & cell) const
{
  return pattern->isInstance(cell);
}

void Define::compile(Jit & jit, Function * f, const Cell & obj) const
{
  if(pattern->isInstance(obj))
  {
    auto cons = obj.as<Cons>()->getCdrCell().as<Cons>();
    jit.compile(f, cons->getCdrCell().as<Cons>()->getCarCell());
    f->appendInstruction(DEFINES, f->dataSize());
    f->appendData(cons->getCarCell());
  }
  else
  {
    // @todo throw exception
  }
}
