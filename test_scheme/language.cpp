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
using UIntegerType = Lisp::UIntegerType;
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
    Object atom = vm.make<UIntegerType>(10);
    REQUIRE(form->isInstance(atom));
    Object func = form->compile(atom);
    REQUIRE(func.isA<Function>());
    REQUIRE(func.getRefCount() == 1u);
    //func.as<Function>()->disassemble(std::cout);
    Object res = vm.eval(func);
    REQUIRE(res.isA<UIntegerType>());
    REQUIRE(res.as<UIntegerType>() == 10);
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
  Object refA(vm.make<Reference>(a, vm.make<UIntegerType>(1)));
  REQUIRE(refA.isA<Reference>());
  Object func = lang->compile(refA);
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  Object res = vm.eval(func);
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 1u);
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
                                  vm.make<UIntegerType>(10))));
    
    vm.getAllocator()->cycle();
  }
  Object res = vm.find("a");
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 10);
  res = vm.eval(lang->compile(vm.make<Symbol>("a")));
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 10);
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
                                              vm.make<UIntegerType>(1))));
  REQUIRE(func.getRefCount() == 1u);
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);
  // (func 2 3)
  Object res = vm.eval(lang->compile(vm.list(func,
                                             vm.make<UIntegerType>(2),
                                             vm.make<UIntegerType>(3))));

  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 1);
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
                                         vm.make<UIntegerType>(1),
                                         vm.make<UIntegerType>(2)));
  Object res = vm.eval(funcall);
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 1);
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
  {
    // (first 1 2)
    Object select = lang->compile(vm.list(first,
                                          vm.make<UIntegerType>(1),
                                          vm.make<UIntegerType>(2)));
    REQUIRE(select.isA<Function>());
    REQUIRE(select.as<Function>()->numArguments() == 0);
    Object res = vm.eval(select);
    REQUIRE(res.isA<UIntegerType>());
    REQUIRE(res.as<UIntegerType>() == 1);
  }
  {
    // (first 3 (second 1 2))
    Object select = lang->compile(vm.list(first,
                                          vm.make<UIntegerType>(3),
                                          vm.list(second,
                                                  vm.make<UIntegerType>(1),
                                                  vm.make<UIntegerType>(2))));
    REQUIRE(select.isA<Function>());
    REQUIRE(select.as<Function>()->numArguments() == 0);
    Object res = vm.eval(select);
    REQUIRE(res.isA<UIntegerType>());
    REQUIRE(res.as<UIntegerType>() == 3);
  }
  {
    // (second (second 1 2) 3)
    Object select = lang->compile(vm.list(second,
                                          vm.list(second,
                                                  vm.make<UIntegerType>(1),
                                                  vm.make<UIntegerType>(2)),
                                          vm.make<UIntegerType>(3)));
    REQUIRE(select.isA<Function>());
    REQUIRE(select.as<Function>()->numArguments() == 0);
    Object res = vm.eval(select);
    REQUIRE(res.isA<UIntegerType>());
    REQUIRE(res.as<UIntegerType>() == 3);
  }
  {
    // (first (first 1 2) 3)
    Object select = lang->compile(vm.list(first,
                                          vm.list(first,
                                                  vm.make<UIntegerType>(1),
                                                  vm.make<UIntegerType>(2)),
                                          vm.make<UIntegerType>(3)));
    REQUIRE(select.isA<Function>());
    REQUIRE(select.as<Function>()->numArguments() == 0);
    Object res = vm.eval(select);
    REQUIRE(res.isA<UIntegerType>());
    REQUIRE(res.as<UIntegerType>() == 1);
  }
  {

    // (second (second 1 2) (first 3 4))
    Object select = lang->compile(vm.list(second,
                                          vm.list(second,
                                                  vm.make<UIntegerType>(1),
                                                  vm.make<UIntegerType>(2)),
                                          vm.list(first,
                                                  vm.make<UIntegerType>(3),
                                                  vm.make<UIntegerType>(4))));
    REQUIRE(select.isA<Function>());
    REQUIRE(select.as<Function>()->numArguments() == 0);
    //select.as<Function>()->disassemble(std::cout);
    Object res = vm.eval(select);
    REQUIRE(res.isA<UIntegerType>());
    REQUIRE(res.as<UIntegerType>() == 3);
 }
}

TEST_CASE("scm_nested_scopes_1", "[Scheme]")
{
  /**
     func=(lambda (a b)
                  (lambda (c d)
                   a))
  */
  Vm vm;
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();
  auto expr = lang->compile(vm.list(vm.make<Symbol>("lambda"),
                                    vm.list(vm.make<Symbol>("a"),
                                            vm.make<Symbol>("b")),
                                    vm.list(vm.make<Symbol>("lambda"),
                                            vm.list(vm.make<Symbol>("c"),
                                                    vm.make<Symbol>("d")),
                                            vm.make<Symbol>("a"))));
  REQUIRE(expr.isA<Function>());
  REQUIRE(expr.as<Function>()->numArguments() == 0);

  auto func = vm.eval(expr);
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);
  REQUIRE(func.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func.as<Function>()->getArgumentTraits(1).isReference());

  auto func2 = vm.eval(func, vm.make<UIntegerType>(1), vm.make<UIntegerType>(2));
  REQUIRE(func2.isA<Function>());
  REQUIRE(func2.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(func2.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func2.as<Function>()->getArgumentTraits(1).isReference());

  auto res = vm.eval(func2, vm.make<UIntegerType>(3), vm.make<UIntegerType>(4));
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 1);
}

TEST_CASE("scm_nested_scopes_2", "[Scheme]")
{
  /*
    (lambda (a b)
      (lambda (c d)
       (lambda (c d) a)))
  */
  Vm vm;
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();

  auto expr = lang->compile(vm.list(vm.make<Symbol>("lambda"),
                                    vm.list(vm.make<Symbol>("a"),
                                            vm.make<Symbol>("b")),
                                    vm.list(vm.make<Symbol>("lambda"),
                                            vm.list(vm.make<Symbol>("c"),
                                                    vm.make<Symbol>("d")),
                                            vm.list(vm.make<Symbol>("lambda"),
                                                    vm.list(vm.make<Symbol>("c"),
                                                            vm.make<Symbol>("d")),
                                                    vm.make<Symbol>("a")))));
  REQUIRE(expr.isA<Function>());
  REQUIRE(expr.as<Function>()->numArguments() == 0);

  auto func = vm.eval(expr);
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 2);
  REQUIRE(func.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func.as<Function>()->getArgumentTraits(1).isReference());


  /*
    (lambda (c d)
      (lambda (c d) a)) */
  auto func1 = vm.eval(func,
                       vm.make<UIntegerType>(1),
                       vm.make<UIntegerType>(2));
  REQUIRE(func1.isA<Function>());
  REQUIRE(func1.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(func1.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func1.as<Function>()->getArgumentTraits(1).isReference());

  /* (lambda (c d) a)) */
  auto func2 = vm.eval(func1,
                       vm.make<UIntegerType>(5),
                       vm.make<UIntegerType>(6));
  REQUIRE(func2.as<Function>()->numArguments() == 2);
  REQUIRE_FALSE(func2.as<Function>()->getArgumentTraits(0).isReference());
  REQUIRE_FALSE(func2.as<Function>()->getArgumentTraits(1).isReference());

  auto res = vm.eval(func2,
                     vm.make<UIntegerType>(3),
                     vm.make<UIntegerType>(4));
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 1);
}


TEST_CASE("scm_car_lambda", "[Scheme]")
{
  /*
   * ((lambda (a b) b)
   *   1 2)
   * -> 2
   */
  Vm vm;
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();
  
  auto expr = lang->compile(vm.list(vm.list(vm.make<Symbol>("lambda"),
                                            vm.list(vm.make<Symbol>("a"),
                                                    vm.make<Symbol>("b")),
                                            vm.make<Symbol>("b")),
                                    vm.make<UIntegerType>(1),
                                    vm.make<UIntegerType>(2)));
  auto res = vm.eval(expr);
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 2);
}

TEST_CASE("scm_car_lambda_2", "[Scheme]")
{
  /*
    (((lambda (a b)
              (lambda (c d) a))
     1 2)
    3 4)

  car:
   (((lambda (a b)
              (lambda (c d) a))
    1 2)
  */
  Vm vm;
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();
    
  auto func = lang->compile(vm.list(vm.list(vm.list(vm.make<Symbol>("lambda"),
                                                    vm.list(vm.make<Symbol>("a"),
                                                            vm.make<Symbol>("b")),
                                                    vm.list(vm.make<Symbol>("lambda"),
                                                            vm.list(vm.make<Symbol>("c"),
                                                                    vm.make<Symbol>("d")),
                                                            vm.make<Symbol>("a"))),
                                            vm.make<UIntegerType>(1),
                                            vm.make<UIntegerType>(2)),
                                    vm.make<UIntegerType>(3),
                                    vm.make<UIntegerType>(4)));
  REQUIRE(func.isA<Function>());
  REQUIRE(func.as<Function>()->numArguments() == 0);
  //func.as<Function>()->disassemble(std::cout);
  auto res = vm.eval(func);
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 1);
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
  Object langObj = vm.make<Language>();
  Language * lang = langObj.as<Language>();
  auto func = lang->compile(vm.list(vm.list(vm.list(vm.make<Symbol>("lambda"),
                                                    vm.list(vm.make<Symbol>("a"),
                                                            vm.make<Symbol>("b")),
                                                    vm.list(vm.make<Symbol>("lambda"),
                                                            vm.list(vm.make<Symbol>("c"),
                                                                    vm.make<Symbol>("d")),
                                                            vm.make<Symbol>("d"))),
                                            vm.make<UIntegerType>(1),
                                            vm.make<UIntegerType>(2)),
                                    vm.make<UIntegerType>(3),
                                    vm.make<UIntegerType>(4)));
  REQUIRE(func.as<Function>()->numArguments() == 0);
  auto res = vm.eval(func);
  REQUIRE(res.isA<UIntegerType>());
  REQUIRE(res.as<UIntegerType>() == 4);
}
