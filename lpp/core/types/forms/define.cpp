#include <lpp/core/types/forms/define.h>
#include <lpp/core/compiler/cons_pattern.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/symbol.h>


using Define = Lisp::Define;
using ConsPattern = Lisp::ConsPattern;


Define::Define()
  : pattern(ConsPattern::make(Type<Symbol>::make(),
                              ConsPattern::make(AnyType::make(),
                                                Type<Nil>::make())))
{}

void Define::pass1(Jit & jit, const Cell & cdr) const
{
  if(pattern->isInstance(cdr))
  {
    jit.pass1(cdr.as<Cons>()->getCdrCell().as<Cons>()->getCarCell());
    jit.instrSize++;
    jit.instrSize++;
    jit.dataSize++;
  }
  else
  {
    // @todo throw exception
  }
}

void Define::pass2(Jit & jit, const Cell & cdr) const
{
  auto f = jit.function.as<Function>();
  jit.pass2(cdr.as<Cons>()->getCdrCell().as<Cons>()->getCarCell());
  f->appendInstruction(DEFINEV, f->dataSize());
  f->appendData(cdr.as<Cons>()->getCarCell());
}

void Define::exception(Jit & jit, const Cell & cdr) const
{
}
