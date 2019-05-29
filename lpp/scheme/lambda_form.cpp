#include <lpp/scheme/lambda_form.h>
#include <lpp/scheme/context.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/opcode.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/forms/list_of.h>
#include <lpp/core/types/forms/type_of.h>
#include <lpp/core/types/forms/cons_of.h>
#include <lpp/core/types/forms/symbol_eq.h>


// allocator
using Guard = Lisp::Allocator::Guard;
using Allocator = Lisp::Allocator;

// types
using Symbol = Lisp::Symbol;
using Idempotent = Lisp::Idempotent;
using Nil = Lisp::Nil;

// forms
using SymbolEq = Lisp::SymbolEq<void>;
using ListOf = Lisp::ListOf;
using SymbolForm = Lisp::TypeOf<Symbol, void>;
using IdempotentForm = Lisp::TypeOf<Idempotent, void>;
using NilForm = Lisp::TypeOf<Nil, void>;

// scheme specific
using LambdaForm = Lisp::Scheme::LambdaForm;

LambdaForm::LambdaForm(Form * _body)
{
  cells.push_back(Cell(_body));
  body = _body;
}

void LambdaForm::init()
{
  auto allocator = getAllocator();
  Guard _lock(allocator);
  lambdaSymbol = makeRoot<SymbolEq<void>>(allocator->make<Symbol>("lambda"));
  argList = makeRoot<ListOf>(make<SymbolForm>(), Context::argumentListForm);
}

inline bool LambdaForm::parse(Function * f0, Cons *cdr) const
{
  Context ctx(getAllocator());
  //@todo add argument list
  if(argList->match(cdr->getCarCell()))
  {
    if(body->match(cdr->getCdrCell()))
    {
      ctx.finalize();
      f0->appendInstruction(RETURNV, f0->dataSize());
      f0->appendData(ctx.getFunctionObject());
      return true;
    }
  }
  return false;
}

bool LambdaForm::match(const Cell & cell) const
{
  bool ret = false;
  Cons * cons = cell.as<Cons>();
  if(cons)
  {
    // ( . )
    if(lambdaSymbol->match(cons->getCarCell()))
    {
      Cons * cdr = cons->getCdrCell().as<Cons>();
      if(cdr)
      {
        ret = parse(Context::getContextStack().back()->getFunction(), cdr);
      }
    }
  }
  return ret;
}

bool LambdaForm::isInstance(const Cell & cell) const
{
  bool ret = false;
  Cons * cons = cell.as<Cons>();
  if(cons)
  {
    // ( . )
    if(lambdaSymbol->isInstance(cons->getCarCell()))
    {
      Cons * cdr = cons->getCdrCell().as<Cons>();
      return (cdr &&
              argList->isInstance(cdr->getCarCell()) &&
              body->isInstance(cdr->getCdrCell()));
    }
  }
  return false;
}
