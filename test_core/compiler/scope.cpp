/******************************************************************************
Copyright (c) 2017-2019, Stefan Wolfsheimer

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
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/object.h>
#include <lpp/core/types/reference.h>
#include <lpp/core/compiler/argument_list.h>
#include <lpp/core/compiler/scope.h>

using Object = Lisp::Object;
using ArgumentList = Lisp::ArgumentList;
using ArgumentReference = Lisp::ArgumentReference;
using Scope = Lisp::Scope;
using SymbolContainer = Lisp::SymbolContainer;

/*
using Symbol = Lisp::Symbol;
using Undefined = Lisp::Undefined;
using IntegerType = Lisp::IntegerType;
using ManagedType = Lisp::ManagedType;
using Reference = Lisp::Reference;
using GarbageCollector = Lisp::GarbageCollector;
*/


TEST_CASE("argument_list_push_scope", "[Env]")
{
  SymbolContainer sc;
  std::shared_ptr<ArgumentList> root;
  auto lst1 = std::make_shared<ArgumentList>(root);
  Object a(sc.make("a"));
  Object b(sc.make("b"));
  Object c(sc.make("c"));
  Object d(sc.make("d"));
  lst1->set(a, ArgumentReference(1));
  lst1->set(b, ArgumentReference(2));
  auto lst2 = std::make_shared<ArgumentList>(lst1);
  lst2->set(c, ArgumentReference(3));
  lst2->set(b, ArgumentReference(4));

  REQUIRE(lst1->find(a).getPos() == 1);
  REQUIRE(lst1->find(b).getPos() == 2);

  //@todo
  //REQUIRE(lst2->find(a).getPos() == 1);
  REQUIRE(lst2->find(c).getPos() == 3);
  REQUIRE(lst2->find(b).getPos() == 4);
}


#if 0
TEST_CASE("scoped_env", "[Env]")
{
  SymbolContainer sc;
  auto env = std::make_shared<Env>();
  Object a(sc.make("a"));
  Object b(sc.make("b"));
  Object c(sc.make("c"));
  Object d(sc.make("d"));
  env->set(a, Object(1));
  env->set(b, Object(2));
  {
    Scope scope1(env);
    REQUIRE(env->find(a).isA<IntegerType>());
    REQUIRE(env->find(a).as<IntegerType>() == 1);
    REQUIRE(env->find(b).isA<IntegerType>());
    REQUIRE(env->find(b).as<IntegerType>() == 2);
    //@todo exception
    REQUIRE(env->find(c).isA<Undefined>());
    REQUIRE(env->find(d).isA<Undefined>());

    env->set(c, Object(3));
    env->set(b, Object(4));
    REQUIRE(env->find(b).isA<IntegerType>());
    REQUIRE(env->find(b).as<IntegerType>() == 4);
    REQUIRE(env->find(c).isA<IntegerType>());
    REQUIRE(env->find(c).as<IntegerType>() == 3);
    {
      Scope scope2(env);
      env->set(b, Object(5));
      env->set(d, Object(6));
      REQUIRE(env->find(a).isA<IntegerType>());
      REQUIRE(env->find(a).as<IntegerType>() == 1);
      REQUIRE(env->find(b).isA<IntegerType>());
      REQUIRE(env->find(b).as<IntegerType>() == 5);
      REQUIRE(env->find(d).isA<IntegerType>());
      REQUIRE(env->find(d).as<IntegerType>() == 6);
    }
    REQUIRE(env->find(b).isA<IntegerType>());
    REQUIRE(env->find(b).as<IntegerType>() == 4);
  }
  REQUIRE(env->find(a).isA<IntegerType>());
  REQUIRE(env->find(a).as<IntegerType>() == 1);
  REQUIRE(env->find(b).isA<IntegerType>());
  REQUIRE(env->find(b).as<IntegerType>() == 2);
  //@todo exception
  REQUIRE(env->find(c).isA<Undefined>());
  REQUIRE(env->find(d).isA<Undefined>());
}
#endif
