#include <lpp/core/compiler/jit.h>
#include <lpp/core/cell.h>
#include <lpp/core/env.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/memory/symbol_container.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/array.h>
#include <lpp/core/types/form.h>
#include <lpp/core/util.h>
#include <lpp/core/compiler/scope.h>

using Jit = Lisp::Jit;
using Object = Lisp::Object;
using Scope = Lisp::Scope;

Jit::Jit(const Jit & rhs) : gc(rhs.gc), sc(rhs.sc), tc(rhs.tc), env(rhs.env)
{
}

Jit::Jit(std::shared_ptr<Allocator> _gc,
         std::shared_ptr<SymbolContainer> _sc,
         std::shared_ptr<TypeContainer> _tc,
         std::shared_ptr<Env> _env) : gc(_gc), sc(_sc), tc(_tc), env(_env)
{
}

void Jit::compileSymbol(Function * f, const Cell & obj)
{
  std::pair<Function*, std::size_t> arg;
  if(scope)
  {
    arg = scope->find(obj, true);
  }
  else
  {
    arg.first = nullptr;
  }
  if(arg.first)
  {
    if(arg.first == scope->getFunction())
    {
      // @todo modify to RETURNR after the function has been completed
      f->appendInstruction(RETURNS, arg.first->numArguments() - arg.second);
    }
    else
    {
      // create shared argument -> return value
      f->appendInstruction(RETURNV, f->dataSize());
      f->appendData(std::move(arg.first->shareArgument(arg.second, gc)));
    }
  }
  else
  {
    // lookup symbol in global scope
    f->appendInstruction(RETURNL, f->dataSize());
    f->appendData(obj);
  }
}

void Jit::compile(Function * f, const Cell & obj)
{
  auto cons = obj.as<Cons>();
  if(cons)
  {
    if(cons->getCarCell().isA<Symbol>())
    {
      // (form ...)
      auto form = env->find(cons->getCarCell()).as<Compilable>();
      if(form)
      {
        form->compile(*this, f, obj);
      }
      else
      {
        // (funcion ...)
      }
    }
    else if(cons->getCarCell().isA<Cons>())
    {
      // ((..) ..)
      //  |
      Jit jit(gc, sc, tc, env);
      InstructionType n = 0;
      forEachCar(cons->getCdrCell(), [&jit, &n, f](const Cell& arg){
          jit.compile(f, arg);
          f->appendInstruction(INCRET);
          n++;
      });
      f->appendInstruction(FUNCALL, n, f->dataSize());
      f->appendData(jit.compile(cons->getCarCell()));
    }
    else if(cons->getCarCell().isA<Function>())
    {
      // (#function ...)
      // @todo match number of arguments
      InstructionType n = 0;
      forEachCar(cons->getCdrCell(), [this, &n, f](const Cell& arg){
          compile(f, arg);
          f->appendInstruction(INCRET);
          n++;
        });
      f->appendInstruction(FUNCALL, n, f->dataSize());
      f->appendData(cons->getCarCell());
    }
  }
  else if(obj.isA<Symbol>())
  {
    compileSymbol(f, obj);
  }
  else
  {
    // value
    f->appendInstruction(RETURNV, f->dataSize());
    f->appendData(obj);
  }
}

Object Jit::compile(const Cell & obj)
{
  Function * f = gc->makeRoot<Function>();
  Object ret(f);
  compile(f, obj);
  f->shrink();
  return std::move(ret);
}
