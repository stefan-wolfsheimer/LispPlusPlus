#include <lpp/core/types/forms/lambda.h>

#include <lpp/core/util.h>
#include <lpp/core/exception.h>
#include <lpp/core/opcode.h>

#include <lpp/core/compiler/cons_pattern.h>
#include <lpp/core/compiler/list_pattern.h>
#include <lpp/core/compiler/jit.h>
#include <lpp/core/compiler/scope.h>

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
  Function * f = jit.gc->makeRoot<Function>();
  if(pattern->isInstance(obj))
  {
    Cons * cons = obj.as<Cons>()->getCdrCell().as<Cons>();
    Scope scope(jit.argumentList);
    std::size_t n = listLength(cons->getCarCell());
    f->setNumArguments(n);
    Lisp::forEachCar(cons->getCarCell(),
                     [&jit, &obj, &f, &n](const Cell & cell) {
                       if(jit.argumentList->isSet(cell))
                       {
                         // argument defined twice
                         // (a b a)
                         throw IllFormed(obj);
                       }
                       jit.argumentList->set(cell,
                                             ArgumentReference(n));
                       f->appendData(cell);
                       n--;
                     });
    // (lambda (...) body)
    //               |
    jit.compile(f, cons->getCdrCell().as<Cons>()->getCarCell());
    f->shrink();

    f0->appendInstruction(RETURNV, f0->dataSize());
    f0->appendData(Object(f));
  }
  else
  {
    throw IllFormed(obj);
  }
}
