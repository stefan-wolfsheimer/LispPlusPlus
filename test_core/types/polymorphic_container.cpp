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
#include <lpp/core/types/polymorphic_container.h>
#include <lpp/core/vm.h>
#include <lpp/core/cell.h>

using PolymorphicContainer = Lisp::PolymorphicContainer;
using Vm = Lisp::Vm;
using Object = Lisp::Object;
using Container = Lisp::Container;
using Cell = Lisp::Cell;
using Cons = Lisp::Cons;

class UserContainer1 : public PolymorphicContainer
{
public:
  UserContainer1(int n, std::shared_ptr<bool> d) : deleted(d), value(n)
  {
  }

  ~UserContainer1()
  {
    *deleted = true;
  }

  virtual void forEachChild(std::function<void(const Cell&)> func) const override
  {
    func(child1);
    func(child2);
  }
    
  virtual bool greyChildren()  override
  {
    child1.grey();
    child2.grey();
    return true;
  }

  virtual void resetGcPosition()  override
  {
  }

  virtual bool recycleNextChild()  override
  {
    child1 = Lisp::nil;
    child2 = Lisp::nil;
    return true;
  }

  Cell child1;
  Cell child2;
  std::shared_ptr<bool> deleted;
  int value;
};

class UserContainer2 : public PolymorphicContainer
{
public:
  UserContainer2(int n, std::shared_ptr<bool> d) : deleted(d), value(n)
  {
  }

  ~UserContainer2()
  {
    *deleted = true;
  }

  virtual void forEachChild(std::function<void(const Cell&)> func) const override
  {
    func(child1);
    func(child2);
  }
    
  virtual bool greyChildren()  override
  {
    child1.grey();
    child2.grey();
    return true;
  }

  virtual void resetGcPosition()  override
  {
  }

  virtual bool recycleNextChild()  override
  {
    child1 = Lisp::nil;
    child2 = Lisp::nil;
    return true;
  }

  Cell child1;
  Cell child2;

  std::shared_ptr<bool> deleted;
  int value;
};

class UserContainer21 : public UserContainer2
{
public:
  UserContainer21(int n, std::shared_ptr<bool> d) : UserContainer2(n, d)
  {
  }

  virtual void forEachChild(std::function<void(const Cell&)> func) const override
  {
    UserContainer2::forEachChild(func);
    func(child3);
  }
    
  virtual bool greyChildren()  override
  {
    UserContainer2::greyChildren();
    child3.grey();
    return true;
  }

  virtual bool recycleNextChild()  override
  {
    UserContainer2::recycleNextChild();
    child3 = Lisp::nil;
    return true;
  }

  Cell child3;
};

TEST_CASE("polymorphic_container", "[PolymorphicContainer]")
{
  auto deleted1 = std::make_shared<bool>(false);
  auto deleted2 = std::make_shared<bool>(false);
  auto deleted21 = std::make_shared<bool>(false);
  {
    Vm vm;
    Object cons;
    {
      auto obj1 = vm.make<UserContainer1>(1, deleted1);
      auto obj2 = vm.make<UserContainer2>(2, deleted2);
      auto obj21 = vm.make<UserContainer21>(21, deleted21);
      REQUIRE(obj1.isA<Container>());
      REQUIRE(obj2.isA<Container>());
      REQUIRE(obj21.isA<Container>());
      REQUIRE(obj1.isA<PolymorphicContainer>());
      REQUIRE(obj2.isA<PolymorphicContainer>());
      REQUIRE(obj21.isA<PolymorphicContainer>());
      
      REQUIRE(obj1.isA<UserContainer1>());
      REQUIRE_FALSE(obj1.isA<UserContainer2>());
      REQUIRE_FALSE(obj1.isA<UserContainer21>());
      
      REQUIRE_FALSE(obj2.isA<UserContainer1>());
      REQUIRE(obj2.isA<UserContainer2>());
      REQUIRE_FALSE(obj2.isA<UserContainer21>());
      
      REQUIRE_FALSE(obj21.isA<UserContainer1>());
      REQUIRE(obj2.isA<UserContainer2>());
      REQUIRE(obj21.isA<UserContainer21>());
      
      REQUIRE(obj1.as<UserContainer1>()->value == 1);
      REQUIRE(obj2.as<UserContainer2>()->value == 2);
      REQUIRE(obj21.as<UserContainer2>()->value == 21);
      REQUIRE(obj21.as<UserContainer21>()->value == 21);

      obj21.as<UserContainer21>()->child3 = obj21;
      obj1.as<UserContainer1>()->child1 = obj2;
      obj1.as<UserContainer1>()->child2 = vm.make<Cons>(obj21, obj2);
      
      cons = vm.make<Cons>(obj1, obj2);
      REQUIRE(cons.isA<Cons>());

    }
    REQUIRE_FALSE(*deleted1);
    REQUIRE_FALSE(*deleted2);
    REQUIRE_FALSE(*deleted21);
  }
  REQUIRE(*deleted1);
  REQUIRE(*deleted2);
  REQUIRE(*deleted21);
}
