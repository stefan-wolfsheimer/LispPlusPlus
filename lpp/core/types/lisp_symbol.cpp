#include "lisp_symbol.h"
#include <lpp/core/lisp_symbol_factory.h>
using Symbol = Lisp::Symbol;

Symbol::~Symbol()
{
  if(factory)
  {
    factory->remove(this); 
  }
}
