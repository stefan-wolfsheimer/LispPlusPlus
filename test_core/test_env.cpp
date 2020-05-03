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
#include <lpp/core/memory/allocator.h>
#include <lpp/core/env.h>
#include <lpp/core/types/reference.h>

using Env = Lisp::Env;
using Object = Lisp::Object;
using Symbol = Lisp::Symbol;
using Undefined = Lisp::Undefined;
using UIntegerType = Lisp::UIntegerType;
using ManagedType = Lisp::ManagedType;
using Reference = Lisp::Reference;
using Allocator = Lisp::Allocator;

TEST_CASE("make_reference", "[Env]")
{
  auto alloc = std::make_shared<Allocator>();
  Object ref_a;
  {
    Object a(alloc->makeRoot<Symbol>("a"));
    REQUIRE(a.isA<Symbol>());
    REQUIRE(a.isA<ManagedType>());
    REQUIRE(a.getRefCount() == 1);
    ref_a = Object(alloc->makeRoot<Reference>(a, Lisp::nil));
    REQUIRE(ref_a.isA<Reference>());
    REQUIRE(a.getRefCount() == 2);
    REQUIRE(ref_a.getRefCount() == 1);
  }
  REQUIRE(ref_a.as<Reference>()->getCarCell().getRefCount() == 1);
}

TEST_CASE("env_make_reference", "[Env]")
{
  Allocator alloc;
  Env env;
  Object a(alloc.makeRoot<Symbol>("a"));
  //@todo exception
  REQUIRE(env.find(a).isA<Undefined>());
  env.set(a, Object(2));
  REQUIRE(env.find(a).isA<UIntegerType>());
  REQUIRE(env.find(a).as<UIntegerType>() == 2);
}
