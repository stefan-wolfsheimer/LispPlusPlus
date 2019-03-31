#include <lpp/scheme/context.h>
#include <assert.h>

using Context = Lisp::Scheme::Context;

Context::Context(Allocator * _alloc)
  : alloc(_alloc), funcObject(_alloc->makeRoot<Function>())
{
  f = funcObject.as<Function>();
  getContextStack().push_back(this);
}

void Context::idempotentForm(const Form * form, const Cell & cell)
{
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->f;
    f->appendInstruction(RETURNV, f->dataSize());
    f->appendData(cell);
  }
}

void Context::symbolForm(const Form * form, const Cell & cell)
{
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->f;
    f->appendInstruction(RETURNL, f->dataSize());
    f->appendData(cell);
  }
}

void Context::defineForm(Form *, const Cell & car, Form *, const Cell & cdr)
{
  if(!Context::getContextStack().empty())
  {
    assert(car.isA<Symbol>());
    Function * f = Context::getContextStack().back()->f;
    f->appendInstruction(DEFINES, f->dataSize());
    f->appendData(car);
  }
}

void Context::argumentListForm(const Cell & car)
{
  //@todo check duplicate
  //@todo lookup in context stack
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->getFunction();
    f->addArgument(car);
  }
}
