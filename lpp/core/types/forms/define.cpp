#include <lpp/core/types/forms/define.h>
#include <lpp/core/compiler/cons_pattern.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/symbol.h>


using Define = Lisp::Define;
using ConsPattern = Lisp::ConsPattern;
using Object = Lisp::Object;


Define::Define()
  : pattern(ConsPattern::make(Type<Symbol>::make(),
                              ConsPattern::make(Type<Symbol>::make(),
                                                ConsPattern::make(AnyType::make(),
                                                                  Type<Nil>::make()))))
{}

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
