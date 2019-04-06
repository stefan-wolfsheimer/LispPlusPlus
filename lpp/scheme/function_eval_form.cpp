#include <lpp/scheme/function_eval_form.h>

using FunctionEvalForm = Lisp::Scheme::FunctionEvalForm;
using Cons = Lisp::Cons;

FunctionEvalForm::FunctionEvalForm(Form * _func,
                                   Form * _expression,
                                   std::function<void()> _funcPush,
                                   std::function<void(std::size_t n, const Cell & f)> func)
  : cb(func), cbPush(_funcPush)
{
  cells.push_back(Cell(_func));
  cells.push_back(Cell(_expression));
  expression = _expression;
  function = _func;
}

void FunctionEvalForm::init()
{
}

bool FunctionEvalForm::isInstance(const Cell & cell) const
{
  Cons * cons = cell.as<Cons>();
  if(cons && function->isInstance(cons->getCarCell()))
  {
    const Cell * c = &cons->getCdrCell();
    while(true)
    {
      if(c->isA<Nil>())
      {
        return true;
      }
      Cons * cons = c->as<Cons>();
      if(cons && expression->isInstance(cons->getCarCell()))
      {
        c = &cons->getCdrCell();
      }
      else
      {
        return false;
      }
    }
  }
  return false;
}

bool FunctionEvalForm::match(const Cell & cell) const
{
  if(!isInstance(cell))
  {
    return false;
  }
  if(cb)
  {
    Cons * cons = cell.as<Cons>();
    std::size_t n = 0;
    const Cell * c = &cons->getCdrCell();
    while(true)
    {
      if(c->isA<Nil>())
      {
        break;
      }
      Cons * cons = c->as<Cons>();
      if(cons && expression->match(cons->getCarCell()))
      {
        cbPush();
        n++;
      }
      c = &cons->getCdrCell();
    }
    cb(n, cons->getCarCell());
  }
  return true;
}

