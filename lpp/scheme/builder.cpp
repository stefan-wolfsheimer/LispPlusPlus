#include <lpp/scheme/builder.h>
#include <lpp/core/types/function.h>
#include <lpp/core/util.h>
#include <lpp/core/opcode.h>

using Builder = Lisp::Scheme::Builder;


// @todo move to separate module
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
  func->appendInstruction(RETURNV, func->dataSize());
  func->appendData(cell);
}

void Builder::reference(const Cell & cell)
{
  func->appendInstruction(RETURNV, func->dataSize());
  func->appendData(cell);
}

void Builder::symbol(const Cell & cell)
{
  func->appendInstruction(RETURNL, func->dataSize());
  func->appendData(cell);

#if 0
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
#endif
  //Context::symbolForm(nullptr, cell);
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


