/*****************************************************************************
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
#include <lpp/core/types/function.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/types/string.h>

#include <lpp/scheme/language.h>

using Vm = Lisp::Vm;

using Language = Lisp::Scheme::Language;
using Object = Lisp::Object;
using IntegerType = Lisp::IntegerType;
using Form = Lisp::Form;
using String = Lisp::String;
using Function = Lisp::Function;
using Symbol = Lisp::Symbol;
using Reference = Lisp::Reference;

/*****************************************
 * Primitives
 *****************************************/
TEST_CASE("scm_primitive", "[Scheme]")
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
    Object res = vm.eval(func);
    REQUIRE(res.isA<IntegerType>());
    REQUIRE(res.as<IntegerType>() == 10);
  }
  {
    Object atom = vm.make<String>("hello");
    REQUIRE(form->isInstance(atom));
    Object func = form->compile(atom);
    REQUIRE(func.isA<Function>());
    REQUIRE(func.getRefCount() == 1u);
    Object res = vm.eval(func);
    REQUIRE(res.isA<String>());
    REQUIRE(res.as<String>()->getCString() == "hello");
  }
}

TEST_CASE("scm_reference", "[Scheme]")
{
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  Object a(vm.make<Symbol>("a"));
  Object refA(vm.make<Reference>(a, vm.make<IntegerType>(1)));
  REQUIRE(refA.isA<Reference>());
  Object func = lang->compile(refA);
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  Object res = vm.eval(func);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1u);
}

/*****************************************
 * Define / symbol lookup
 *****************************************/
TEST_CASE("scm_symbol_lookup", "[Scheme]")
{
  using Undefined = Lisp::Undefined;
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  Object func = formObj.as<Language>()->compile(vm.make<Symbol>("a"));
  REQUIRE(func.isA<Function>());
  REQUIRE(func.getRefCount() == 1u);
  // @todo exception if unbound
  Object res = vm.eval(func);
  REQUIRE(res.isA<Undefined>());
}

TEST_CASE("scm_define_primitive", "[Scheme]")
{
  using Undefined = Lisp::Undefined;
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  {

    vm.eval(lang->compile(vm.list(vm.make<Symbol>("define"),
                                  vm.make<Symbol>("a"),
                                  vm.make<IntegerType>(10))));
    
    vm.getAllocator()->cycle();
  }
  Object res = vm.find("a");
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 10);
  res = vm.eval(lang->compile(vm.make<Symbol>("a")));
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 10);
}

/*****************************************
 * Lambda
 *****************************************/
TEST_CASE("scm_lambda_constant", "[Scheme]")
{
  // ((lambda (a b) 1) -> #Function
  Vm vm;
  Object formObj = vm.make<Language>();
  Language * lang = formObj.as<Language>();
  Object func = vm.eval(lang->compile(vm.list(vm.make<Symbol>("lambda"),
                                              vm.list(vm.make<Symbol>("a"),
                                                      vm.make<Symbol>("b")),
                                              vm.make<IntegerType>(1))));
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);

  // (func 2 3)
  Object res = vm.eval(lang->compile(vm.list(func,
                                             vm.make<IntegerType>(2),
                                             vm.make<IntegerType>(3))));

  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("scm_lambda_lookup", "[Scheme]")
{
  // (define f (lambda (a b) a))
  Vm vm;
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();
  vm.eval(lang->compile(vm.list(vm.make<Symbol>("define"),
                                vm.make<Symbol>("f"),
                                vm.list(vm.make<Symbol>("lambda"),
                                        vm.list(vm.make<Symbol>("a"),
                                                vm.make<Symbol>("b")),
                                        vm.make<Symbol>("a")))));
  Object lu = vm.find("f");
  REQUIRE(lu.isA<Function>());
  Object funcall = lang->compile(vm.list(vm.make<Symbol>("f"),
                                         vm.make<IntegerType>(1),
                                         vm.make<IntegerType>(2)));
  Object res = vm.eval(funcall);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("scm_nested_lambdas", "[Scheme]")
{
  // (lambda (a b) a) -> #Function
  // (lambda (a b) b) -> #Function
  Vm vm;
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();
  Object first = vm.eval(lang->compile(vm.list(vm.make<Symbol>("lambda"),
                                               vm.list(vm.make<Symbol>("a"),
                                                       vm.make<Symbol>("b")),
                                               vm.make<Symbol>("a"))));
  Object second = vm.eval(lang->compile(vm.list(vm.make<Symbol>("lambda"),
                                                vm.list(vm.make<Symbol>("a"),
                                                        vm.make<Symbol>("b")),
                                                vm.make<Symbol>("b"))));
  REQUIRE(first.isA<Function>());
  REQUIRE(first.as<Function>()->numArguments() == 2);
  REQUIRE(second.isA<Function>());
  REQUIRE(second.as<Function>()->numArguments() == 2);

  // (first 1 2)
  Object res = vm.eval(lang->compile(vm.list(first,
                                             vm.make<IntegerType>(1),
                                             vm.make<IntegerType>(2))));
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);

  // (first 3 (second 1 2))
  Object select = lang->compile(vm.list(first,
                                        vm.make<IntegerType>(3),
                                        vm.list(second,
                                                vm.make<IntegerType>(1),
                                                vm.make<IntegerType>(2))));
  REQUIRE(select.isA<Function>());
  REQUIRE(select.as<Function>()->numArguments() == 0);
#if 0
  select.as<Function>()->disassemble(std::cout);


  res = vm.eval(select);

  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 3);

  // (second (second 1 2) (first 3 4))
  select = vm.compile(lang,
                      vm.list(second,
                              vm.list(second,
                                      vm.make<IntegerType>(1),
                                      vm.make<IntegerType>(2)),
                              vm.list(first,
                                      vm.make<IntegerType>(3),
                                      vm.make<IntegerType>(4))));

  REQUIRE(select.isA<Function>());
  REQUIRE(select.as<Function>()->numArguments() == 0);

  res = vm.evalAndReturn(select);
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 3);
#endif
}


#if 0
TEST_CASE("scm_nested_scopes_1", "[Scheme]")
{
  Vm vm;
  Object lang = vm.make<Language>();
  std::size_t initStackSize = vm.stackSize();
  /**
     func=(lambda (a b)
                  (lambda (c d))
                  a)
  */
  Object func = vm.compileAndEval(lang,
                                  vm.list(vm.make<Symbol>("lambda"),
                                          vm.list(vm.make<Symbol>("a"),
                                                  vm.make<Symbol>("b")),
                                          vm.list(vm.make<Symbol>("lambda"),
                                                  vm.list(vm.make<Symbol>("c"),
                                                          vm.make<Symbol>("d")),
                                                  vm.make<Symbol>("a"))));
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);
  REQUIRE(func.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func.as<Function>()->getArgumentTraits(1).isReference());
  /* (func 1 2) -> (lambda (c d) 1)
   */
  auto func1 = vm.compileAndEval(lang,
                                 vm.list(func,
                                         vm.make<IntegerType>(1),
                                         vm.make<IntegerType>(2)));
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(func1.isA<Function>());
  auto res = vm.compileAndEval(lang,
                               vm.list(func1,
                                       vm.make<IntegerType>(3),
                                       vm.make<IntegerType>(4)));
  REQUIRE(vm.stackSize() == initStackSize);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("scm_nested_scopes_2", "[Scheme]")
{
  /*
    (lambda (a b)
      (lambda (c d)
       (lambda (c d) a)))
  */
  Vm vm;
  Object lang = vm.make<Language>();
  std::size_t initStackSize = vm.stackSize();
  Object func = vm.compileAndEval(lang,
                                  vm.list(vm.make<Symbol>("lambda"),
                                          vm.list(vm.make<Symbol>("a"),
                                                  vm.make<Symbol>("b")),
                                          vm.list(vm.make<Symbol>("lambda"),
                                                  vm.list(vm.make<Symbol>("c"),
                                                          vm.make<Symbol>("d")),
                                                  vm.list(vm.make<Symbol>("lambda"),
                                                          vm.list(vm.make<Symbol>("c"),
                                                                  vm.make<Symbol>("d")),
                                                          vm.make<Symbol>("a")))));
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);
  REQUIRE(func.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func.as<Function>()->getArgumentTraits(1).isReference());
  auto func1 = vm.compileAndEval(lang,
                                 vm.list(func,
                                         vm.make<IntegerType>(1),
                                         vm.make<IntegerType>(2)));
  /*
    (lambda (c d)
      (lambda (c d) a)) */
  REQUIRE(func1.isA<Function>());
  REQUIRE(func1.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(func1.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func1.as<Function>()->getArgumentTraits(1).isReference());
  auto func2 = vm.compileAndEval(lang,
                                 vm.list(func1,
                                         vm.make<IntegerType>(5),
                                         vm.make<IntegerType>(6)));
  /* (lambda (c d) a)) */
  REQUIRE(func2.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(func2.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func2.as<Function>()->getArgumentTraits(1).isReference());

  auto res = vm.compileAndEval(lang,
                               vm.list(func2,
                                       vm.make<IntegerType>(3),
                                       vm.make<IntegerType>(4)));
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("scm_car_lambda", "[Scheme]")
{
  /*
   * ((lambda (a b) b)
   *   1 2)
   * -> 2
   */
  Vm vm;
  Object lang = vm.make<Language>();
  std::size_t initStackSize = vm.stackSize();
  Object res = vm.compileAndEval(lang,
                                 vm.list(vm.list(vm.make<Symbol>("lambda"),
                                                 vm.list(vm.make<Symbol>("a"),
                                                         vm.make<Symbol>("b")),
                                                 vm.make<Symbol>("b")),
                                         vm.make<IntegerType>(1),
                                         vm.make<IntegerType>(2)));
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 2);
}

TEST_CASE("scm_car_lambda_2", "[Scheme]")
{
  /*
   * car:
   * ((lambda (a b)
   *           (lambda (c d) a)) 1 2)
   *
   * (((lambda (a b)
   *           (lambda (c d) a))) 3 4)
   * -> 1
   */
  Vm vm;
  Object lang = vm.make<Language>();
  std::size_t initStackSize = vm.stackSize();
  Object func = vm.compile(lang,
                            vm.list(vm.list(vm.list(vm.make<Symbol>("lambda"),
                                                    vm.list(vm.make<Symbol>("a"),
                                                            vm.make<Symbol>("b")),
                                                    vm.list(vm.make<Symbol>("lambda"),
                                                            vm.list(vm.make<Symbol>("c"),
                                                                    vm.make<Symbol>("d")),
                                                            vm.make<Symbol>("a"))),
                                            vm.make<IntegerType>(1),
                                            vm.make<IntegerType>(2)),
                                    vm.make<IntegerType>(3),
                                    vm.make<IntegerType>(4)));
  Object res = vm.evalAndReturn(func);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 1);
}

TEST_CASE("scm_car_lambda_3", "[Scheme]")
{
  /*
   * car:
   * ((lambda (a b)
   *           (lambda (c d) d)) 1 2)
   *
   * (((lambda (a b)
   *           (lambda (c d) d))) 3 4)
   * -> 4
   */
  Vm vm;
  Object lang = vm.make<Language>();
  std::size_t initStackSize = vm.stackSize();
  Object func = vm.compile(lang,
                            vm.list(vm.list(vm.list(vm.make<Symbol>("lambda"),
                                                    vm.list(vm.make<Symbol>("a"),
                                                            vm.make<Symbol>("b")),
                                                    vm.list(vm.make<Symbol>("lambda"),
                                                            vm.list(vm.make<Symbol>("c"),
                                                                    vm.make<Symbol>("d")),
                                                            vm.make<Symbol>("d"))),
                                            vm.make<IntegerType>(1),
                                            vm.make<IntegerType>(2)),
                                    vm.make<IntegerType>(3),
                                    vm.make<IntegerType>(4)));
  Object res = vm.evalAndReturn(func);
  REQUIRE(res.isA<IntegerType>());
  REQUIRE(res.as<IntegerType>() == 4);
}

#endif
