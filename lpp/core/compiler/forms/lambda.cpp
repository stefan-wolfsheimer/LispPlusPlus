#include <lpp/core/compiler/forms/lambda.h>

#include <lpp/core/util.h>
#include <lpp/core/exception.h>
#include <lpp/core/opcode.h>

#include <lpp/core/compiler/cons_pattern.h>
#include <lpp/core/compiler/list_pattern.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/compiler/scope_guard.h>

#include <lpp/core/gc/garbage_collector.h>

#include <lpp/core/types/cons.h>
#include <lpp/core/types/function.h>


using Lambda = Lisp::Lambda;

Lambda::Lambda() : pattern(ConsPattern::make(Type<Symbol>::make(),
                                             ConsPattern::make(ListPattern::make(Type<Symbol>::make()),
                                                               ConsPattern::make(AnyType::make(),
                                                                                 Type<Nil>::make()))))
{}

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
