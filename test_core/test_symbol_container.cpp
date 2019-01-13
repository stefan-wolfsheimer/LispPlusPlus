/******************************************************************************
Copyright (c) 2018, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/
#include <lpp/core/object.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/types/symbol.h>

#include <catch.hpp>

using Symbol = Lisp::Symbol;
using SymbolContainer = Lisp::SymbolContainer;
using Object = Lisp::Object;

TEST_CASE("symbol_life_cycle", "[SymbolFactory]")
{
  SymbolContainer container;
  Symbol * psymb = container.make("symb1");
  REQUIRE(psymb->getRefCount() == 0);
  Object symb1(psymb);
  REQUIRE(psymb->getRefCount() == 1);
  REQUIRE(symb1.isA<Symbol>());
  REQUIRE(symb1.as<Symbol>() == psymb);
  REQUIRE(symb1.as<Lisp::Symbol>()->getName() == "symb1");
  {
    Object symb1Copy1(symb1);
    REQUIRE(symb1Copy1.isA<Symbol>());
    REQUIRE(symb1Copy1.as<Lisp::Symbol>() == psymb);
    REQUIRE(psymb->getRefCount() == 2);
    Object symb1Copy2(container.make("symb1"));
    REQUIRE(symb1Copy2.isA<Symbol>());
    REQUIRE(psymb->getRefCount() == 3);
    REQUIRE(symb1Copy2.as<Lisp::Symbol>() == psymb);
  }
  REQUIRE(psymb->getRefCount() == 1);
}
