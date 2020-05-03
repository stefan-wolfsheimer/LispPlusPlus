/******************************************************************************
Copyright (c) 2018-2019, Stefan Wolfsheimer

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
#include <catch.hpp>
#include <lpp/core/vm.h>
#include <lpp/core/util.h>

using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Cell = Lisp::Cell;
using Cons = Lisp::Cons;
using UIntegerType = Lisp::UIntegerType;


TEST_CASE("is_a_list", "[Util]")
{
  Vm vm;
  REQUIRE(Lisp::isAList(Lisp::nil));
  REQUIRE(Lisp::isAList(vm.list(Object(1),
                                Object(2),
                                Object(3))));
  REQUIRE_FALSE(Lisp::isAList(Object(1)));
  REQUIRE_FALSE(Lisp::isAList(vm.make<Cons>(Object(1),
                                            vm.make<Cons>(Object(2),
                                                          Object(3)))));
}

TEST_CASE("list_length", "[Util]")
{
  Vm vm;
  REQUIRE(Lisp::listLength(Lisp::nil) == 0);
  REQUIRE(Lisp::listLength(vm.list(Object(1),
                                   Object(2),
                                   Object(3))) == 3);
  REQUIRE_THROWS(Lisp::listLength(Object(1)));
  REQUIRE_THROWS(Lisp::listLength(vm.make<Cons>(Object(1),
                                                vm.make<Cons>(Object(2),
                                                              Object(3)))));
}

TEST_CASE("for_each_car", "[Util]")
{
  Vm vm;
  {
    UIntegerType n = 0;
    Lisp::forEachCar(Lisp::nil,
                     [&n](const Cell & cell) { n += cell.as<UIntegerType>(); });
    REQUIRE(n == 0);
  }
  {
    UIntegerType n = 0;
    Lisp::forEachCar(vm.list(Object(1),
                             Object(2),
                             Object(3)),
                     [&n](const Cell & cell) { n += cell.as<UIntegerType>(); });
    REQUIRE(n == 6);
  }
  {
    UIntegerType n = 0;
    REQUIRE_THROWS(Lisp::forEachCar(Object(1),
                                    [&n](const Cell & cell) { n += cell.as<UIntegerType>(); }));
                 
    REQUIRE_THROWS(Lisp::forEachCar(vm.make<Cons>(Object(1),
                                                  vm.make<Cons>(Object(2),
                                                                Object(3))),
                                    [&n](const Cell & cell) { n += cell.as<UIntegerType>(); }));
  }
}
