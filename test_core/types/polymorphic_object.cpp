/******************************************************************************
Copyright (c) 2019, Stefan Wolfsheimer

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
#include <lpp/core/types/polymorphic_object.h>
#include <lpp/core/gc/garbage_collector.h>

using PolymorphicObject = Lisp::PolymorphicObject;
using GarbageCollector = Lisp::GarbageCollector;
using Object = Lisp::Object;
using ManagedType = Lisp::ManagedType;
using PolymorphicObject = Lisp::PolymorphicObject;

class UserObject1 : public PolymorphicObject
{
public:
  UserObject1(int n, std::shared_ptr<bool> d) : deleted(d), value(n)
  {
  }

  ~UserObject1()
  {
    *deleted = true;
  }
  std::shared_ptr<bool> deleted;
  int value;
};

class UserObject2 : public PolymorphicObject
{
public:
  UserObject2(int n, std::shared_ptr<bool> d) : deleted(d), value(n)
  {
  }

  ~UserObject2()
  {
    *deleted = true;
  }
  std::shared_ptr<bool> deleted;
  int value;
};

class UserObject21 : public UserObject2
{
public:
  UserObject21(int n, std::shared_ptr<bool> d) : UserObject2(n, d)
  {
  }
};

  
TEST_CASE("polymorphic_object", "[Types]")
{
  auto deleted1 = std::make_shared<bool>(false);
  auto deleted2 = std::make_shared<bool>(false);
  auto deleted21 = std::make_shared<bool>(false);
  {
    auto coll = std::make_shared<GarbageCollector>(8, 1, 1);
    /* @todo manage ManagedTypes in garbage collector (to be consistent)*/
    Object obj1(new UserObject1(1, deleted1));
    Object obj2(new UserObject2(2, deleted2));
    Object obj21(new UserObject21(21, deleted21));
    REQUIRE(obj1.isA<ManagedType>());
    REQUIRE(obj2.isA<ManagedType>());
    REQUIRE(obj21.isA<ManagedType>());
    REQUIRE(obj1.isA<PolymorphicObject>());
    REQUIRE(obj2.isA<PolymorphicObject>());
    REQUIRE(obj21.isA<PolymorphicObject>());

    REQUIRE(obj1.isA<UserObject1>());
    REQUIRE_FALSE(obj1.isA<UserObject2>());
    REQUIRE_FALSE(obj1.isA<UserObject21>());

    REQUIRE_FALSE(obj2.isA<UserObject1>());
    REQUIRE(obj2.isA<UserObject2>());
    REQUIRE_FALSE(obj2.isA<UserObject21>());

    REQUIRE_FALSE(obj21.isA<UserObject1>());
    REQUIRE(obj2.isA<UserObject2>());
    REQUIRE(obj21.isA<UserObject21>());

    REQUIRE(obj1.as<UserObject1>()->value == 1);
    REQUIRE(obj2.as<UserObject2>()->value == 2);
    REQUIRE(obj21.as<UserObject2>()->value == 21);
    REQUIRE(obj21.as<UserObject21>()->value == 21);
  }
  REQUIRE(*deleted1);
  REQUIRE(*deleted2);
  REQUIRE(*deleted21);
}
