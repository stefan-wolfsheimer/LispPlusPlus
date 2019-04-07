#include <lpp/scheme/context.h>
#include <lpp/core/util.h>
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

void Context::referenceForm(const Form * form, const Cell & cell)
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
  std::vector<Lisp::Scheme::Context*> & stack(Context::getContextStack());
  if(!stack.empty())
  {
    auto itr = stack.rbegin();
    Function * f = (*itr)->f;
    std::size_t pos = f->getArgumentPos(cell);
    if(pos != Function::notFound)
    {
      // this symbol is bound to the last function
      f->appendInstruction(RETURNS, f->numArguments() - pos);
      return;
    }
    ++itr;
    for(; itr != stack.rend(); ++itr)
    {
      // create shared argument from parent scope
      std::size_t pos = (*itr)->f->getArgumentPos(cell);
      if(pos != Function::notFound)
      {
        f->appendInstruction(RETURNV, f->dataSize());
        f->appendData(std::move((*itr)->f->shareArgument(pos)));
        return;
      }
    }
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

void Context::argumentListForm(const Cell & lst)
{
  //@todo check duplicate
  //@todo lookup in context stack
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->getFunction();
    Lisp::forEachCar(lst, [f](const Cell & cell) {
        f->addArgument(cell);
     });
  }
}

void Context::procedureCallArgumentForm()
{
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->getFunction();
    f->appendInstruction(INCRET);
  }
}

void Context::procedureCallForm(std::size_t n)
{
  if(!Context::getContextStack().empty())
  {
    Function * f = Context::getContextStack().back()->getFunction();
    f->appendInstruction(FUNCALL, n);
  }
}
