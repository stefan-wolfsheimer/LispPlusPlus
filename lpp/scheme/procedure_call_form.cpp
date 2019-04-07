#include <lpp/scheme/procedure_call_form.h>

using ProcedureCallForm = Lisp::Scheme::ProcedureCallForm;
using Cons = Lisp::Cons;

ProcedureCallForm::ProcedureCallForm(Form * _expression,
                                     std::function<void()> _funcPush,
                                     std::function<void(std::size_t n)> _cb)
  : cb(_cb), cbPush(_funcPush)
{
  cells.push_back(Cell(_expression));
  expression = _expression;
}

void ProcedureCallForm::init()
{
}

bool ProcedureCallForm::isInstance(const Cell & cell) const
{
  const Cell * c = &cell;
  while(true)
  {
    if(c->isA<Nil>())
    {
      return true;
    }
    Cons * cons = c->as<Cons>();
    if(cons)
    {
      if(!expression->isInstance(cons->getCarCell()))
      {
        return false;
      }
      c = &cons->getCdrCell();
    }
    else
    {
      return false;
    }
  }
  return false;
}

bool ProcedureCallForm::match(const Cell & cell) const
{
  if(!isInstance(cell))
  {
    return false;
  }
  if(cb)
  {
    const Cell * c = &cell;
    std::size_t n = 0;
    while(true)
    {
      if(c->isA<Nil>())
      {
        cb(n);
        return true;
      }
      Cons * cons = c->as<Cons>();
      if(expression->match(cons->getCarCell()))
      {
        cbPush();
        n++;
      }
      c = &cons->getCdrCell();
    }
  }
  return true;
}

