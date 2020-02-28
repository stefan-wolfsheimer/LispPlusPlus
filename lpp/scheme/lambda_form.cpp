#include <lpp/scheme/lambda_form.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/cons.h>


// lisp
using Cell = Lisp::Cell;

// types
using Function = Lisp::Function;
using Cons = Lisp::Cons;

// scheme
using Builder = Lisp::Scheme::Builder;
using LambdaForm = Lisp::Scheme::LambdaForm;


LambdaForm::LambdaForm(Lisp::FormBuilder<Builder> * _argList,
                       Lisp::FormBuilder<Builder> * _body)
{
  cells.push_back(Cell(_argList));
  cells.push_back(Cell(_body));
  argList = _argList;
  body = _body;
}

bool LambdaForm::isInstance(const Cell & cell) const
{
  assert(cell.isA<Cons>());
  return
    argList->isInstance(cell.as<Cons>()->getCarCell()) &&
    body->isInstance(cell.as<Cons>()->getCdrCell());
}

bool LambdaForm::match(const Cell & cell, Builder & parentBuilder) const
{
  assert(cell.isA<Cons>());
  Builder builder(getAllocator(), &parentBuilder);
  if(argList->match(cell.as<Cons>()->getCarCell(), builder))
  {
    // current implementation: only one expression in the lambda body
    if(body->match(cell.as<Cons>()->getCdrCell(), builder))
    {
      builder.finalize();
      parentBuilder.getFunction()->addPUSHV(builder.getFunctionObject());
      return true;
    }
  }
  return false;
}
