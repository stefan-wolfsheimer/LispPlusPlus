#pragma once
#include <lpp/core/types/form.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/symbol.h>
#include <lpp/core/types/function.h>
#include <lpp/core/compiler/jit.h>

namespace Lisp
{

  class Define : public Form
  {
    virtual void pass1(Jit & jit, const Cell & cdr) const override
    {
      auto cons = cdr.as<Cons>();
      if(cons)
      {
        if(cons->getCarCell().isA<Symbol>())
        {
          auto cons2 = cons->getCdrCell().as<Cons>();
          if(cons2)
          {
            if(cons2->getCdrCell().isA<Nil>())
            {
              jit.pass1(cons2->getCarCell());
              jit.instrSize++;
              jit.instrSize++;
              jit.dataSize++;
            }
            else
            {
              // @todo throw exception
            }
          }
          else
          {
            // @todo throw exception
          }
        }
        else
        {
          // @todo throw exception
        }
      }
      else
      {
        // @todo throw exception
      }
    }

    virtual void pass2(Jit & jit, const Cell & cdr) const override
    {
      auto cons = cdr.as<Cons>();
      auto f = jit.function.as<Function>();
      jit.pass2(cons->getCdrCell().as<Cons>()->getCarCell());
      f->appendInstruction(DEFINEV, f->dataSize());
      f->appendData(cons->getCarCell());
    }

    virtual void exception(Jit & jit, const Cell & cdr) const override
    {
    }
  };
}
