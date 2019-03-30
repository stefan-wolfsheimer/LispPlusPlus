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
using ConsOf = Lisp::ConsOf;
using ListOf = Lisp::ListOf;
using SymbolForm = Lisp::TypeOf<Symbol>;
using NilForm = Lisp::TypeOf<Nil>;
using AnyForm = Lisp::TypeOf<Any>;


Define::Define() : pattern(nullptr)
{
}

void Define::init()
{
  Lisp::Allocator::Guard _lock(*getCollector());
  pattern = makeRoot<ConsOf>(make<SymbolForm>(),
                             make<ConsOf>(make<SymbolForm>(),
                                          make<ConsOf>(make<AnyForm>(),
                                                       make<NilForm>())));
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
