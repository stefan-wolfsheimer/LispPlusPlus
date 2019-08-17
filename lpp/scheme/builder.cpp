#include <lpp/scheme/builder.h>
#include <lpp/core/types/function.h>
#include <lpp/core/util.h>
#include <lpp/core/opcode.h>

using Builder = Lisp::Scheme::Builder;

Builder::Builder(Allocator * _allocator)
  : allocator(_allocator),
    funcObject(_allocator->makeRoot<Function>())
{
  func = funcObject.as<Function>();
}

void Builder::finalize()
{
  func->shrink();
}

void Builder::idempotent(const Cell & cell)
{
  func->addRETURNV(cell);
}

void Builder::reference(const Cell & cell)
{
  func->addRETURNV(cell);
}

void Builder::symbol(const Cell & cell)
{
  // @todo look in higher functions' arguments
  std::size_t argPos = func->getArgumentPos(cell);
  if(argPos == Function::notFound)
  {
    func->addRETURNL(cell);
  }
  else
  {
    assert(func->numArguments() > argPos);
    func->addRETURNS(func->numArguments() - argPos);
  }
}

void Builder::define(const Cell & car, const Cell & cdr)
{
  func->addDEFINES(car);
}

void Builder::lambdaArgument(const Cell & arg)
{
  func->addArgument(arg);
}

void Builder::funcall(const Cell & lst)
{
  std::size_t l = listLength(lst);
  if(l == 0)
  {
    func->addRETURNV(Lisp::nil);
  }
  else
  {
    func->addFUNCALL(l - 1);
  }
}


