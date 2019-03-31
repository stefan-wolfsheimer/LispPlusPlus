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
#include <catch.hpp>
#include <lpp/core/vm.h>
#include <lpp/core/types/lisp_string.h>
#include <lpp/core/types/function.h>

#include <lpp/scheme/language.h>

using Vm = Lisp::Vm;

using Language = Lisp::Scheme::Language;
using Object = Lisp::Object;
using IntegerType = Lisp::IntegerType;
using Form = Lisp::Form;
using String = Lisp::String;
using Function = Lisp::Function;
using Symbol = Lisp::Symbol;


TEST_CASE("scm_primitive", "[Language]")
{
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * form = formObj.as<Language>();
  {
    Object atom = vm.make<IntegerType>(10);
    REQUIRE(form->isInstance(atom));
    Object func = form->compile(atom);
    REQUIRE(func.isA<Function>());
    REQUIRE(func.getRefCount() == 1u);
    Object res = vm.evalAndReturn(func.as<Function>());
    REQUIRE(res.isA<IntegerType>());
    REQUIRE(res.as<IntegerType>() == 10);
  }
  {
    Object atom = vm.make<String>("hello");
    REQUIRE(form->isInstance(atom));
    Object func = form->compile(atom);
    REQUIRE(func.isA<Function>());
    REQUIRE(func.getRefCount() == 1u);
    Object res = vm.evalAndReturn(func.as<Function>());
    REQUIRE(res.isA<String>());
    REQUIRE(res.as<String>()->getCString() == "hello");
  }
}

TEST_CASE("scm_symbol_lookup", "[Scheme]")
{
  using Undefined = Lisp::Undefined;
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  std::size_t stackSize = vm.stackSize();
  Object func = formObj.as<Language>()->compile(vm.make<Symbol>("a"));
  REQUIRE(func.isA<Function>());
  REQUIRE(func.getRefCount() == 1u);
  // @todo exception if unbound
  Object res = vm.evalAndReturn(func.as<Function>());
  REQUIRE(res.isA<Undefined>());
  REQUIRE(stackSize == vm.stackSize());
}

TEST_CASE("scm_define_primitive", "[Scheme]")
{
  using Undefined = Lisp::Undefined;
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  std::size_t stackSize = vm.stackSize();
  lang->compileAndEval(vm,
                       vm.list(vm.make<Symbol>("define"),
                               vm.make<Symbol>("a"),
                               vm.make<IntegerType>(10)));
  Object res = lang->compileAndEval(vm,
                                    vm.make<Symbol>("a"));
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 10);
  REQUIRE(stackSize == vm.stackSize());
}

TEST_CASE("scm_lambda_constant", "[Scheme]")
{
  // (lambda (a b) 1) -> #Function
  Vm vm;
  std::size_t initStackSize = vm.stackSize();
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  Object func = lang->compileAndEval(vm,
                                     vm.list(vm.make<Symbol>("lambda"),
                                             vm.list(vm.make<Symbol>("a"), vm.make<Symbol>("b")),
                                             vm.make<IntegerType>(1)));
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);
  REQUIRE(vm.stackSize() == initStackSize);

  // (func 2 3)
  REQUIRE(vm.stackSize() == initStackSize);
  // @todo move to lang->compileAndEval (requires (<funcobj> ... ) form)
  Object res = vm.compileAndEval(vm.list(func,
                                         vm.make<IntegerType>(2),
                                         vm.make<IntegerType>(3)));
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);

}

