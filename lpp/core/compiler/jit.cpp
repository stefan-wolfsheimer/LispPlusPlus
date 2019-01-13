#include <lpp/core/compiler/jit.h>
#include <lpp/core/cell.h>
#include <lpp/core/env.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/function.h>
#include <lpp/core/types/array.h>
#include <lpp/core/types/form.h>

using Jit = Lisp::Jit;
using Object = Lisp::Object;


Jit::Jit(std::shared_ptr<GarbageCollector> _gc,
         std::shared_ptr<SymbolContainer> _sc,
         std::shared_ptr<Env> _env) : gc(_gc), sc(_sc), env(_env)
{
  instrSize = 0;
  dataSize = 0;
}

void Jit::pass1(const Cell & obj)
{
  auto cons = obj.as<Cons>();
  if(cons)
  {
    if(cons->getCarCell().isA<Symbol>())
    {
      auto form = env->find(cons->getCarCell()).as<Form>();
      if(form)
      {
        form->pass1(*this, cons->getCdrCell());
      }
      else
      {
        // @todo throw if undefined
      }
    }
    else
    {
      // @todo eval
    }
  }
  else if(obj.isA<Symbol>())
  {
    // symbol lookup
    instrSize++;
    instrSize++;
    dataSize++;
  }
  else
  {
    //idempotent
    instrSize++;
    instrSize++;
    dataSize++;
  }
}

void Jit::pass2(const Cell & obj)
{
  auto f = function.as<Function>();
  if(!f)
  {
    f = gc->makeRoot<Function>(instrSize, dataSize);
    function = Object(f);
  }
  auto cons = obj.as<Cons>();
  if(cons)
  {
    if(cons->getCarCell().isA<Symbol>())
    {
      env
        ->find(cons->getCarCell()).as<Form>()
        ->pass2(*this, cons->getCdrCell());
    }
  }
  else if(obj.isA<Symbol>())
  {
    // @todo dialect variations: check if reference is bound
    f->appendInstruction(LOOKUP, f->dataSize());
    f->appendData(obj);
  }
  else
  {
    f->appendInstruction(SETV, f->dataSize());
    f->appendData(obj);
  }
}

