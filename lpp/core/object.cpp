/******************************************************************************
Copyright (c) 2017, Stefan Wolfsheimer

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
#include <assert.h>
#include <lpp/core/object.h>
#include <lpp/core/types/cons.h>
#include <lpp/core/types/container.h>
#include <lpp/core/gc/garbage_collector.h>

using Object = Lisp::Object;
using Nil = Lisp::Nil;

Object::Object(const Object & rhs) : Cell(rhs)
{
  if(rhs.isA<Lisp::Cons>())
  {
    static_cast<Cons*>(data.ptr)->root();
  }
  else if(rhs.isA<Lisp::Container>())
  {
    static_cast<Container*>(data.ptr)->root();
  }
}

Object::Object(const Cell & rhs) : Cell(rhs)
{
  if(rhs.isA<Cons>())
  {
    static_cast<Cons*>(data.ptr)->root();
  }
  else if(rhs.isA<Container>())
  {
    static_cast<Container*>(data.ptr)->root();
  }
}

Object::~Object()
{
  unsetCons();
}

Object & Lisp::Object::operator=(const Object & rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  if(rhs.isA<Lisp::Cons>())
  {
    assert(rhs.isRoot());
    assert(rhs.getRefCount() > 0u);
    assert(rhs.checkIndex());
    data.ptr = rhs.as<Cons>();
    static_cast<Cons*>(data.ptr)->incRefCount();
  }
  else if(rhs.isA<Lisp::Container>())
  {
    assert(rhs.isRoot());
    assert(rhs.getRefCount() > 0u);
    assert(rhs.checkIndex());
    static_cast<Container*>(data.ptr)->incRefCount();
  }
  else if(rhs.isA<Lisp::ManagedType>())
  {
    Cell::init(rhs.as<ManagedType>(), rhs.getTypeId());
  }
  return *this;
}


Lisp::Object & Lisp::Object::operator=(Object && rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  data = rhs.data;
  rhs.typeId = TypeTraits<Nil>::typeId;
  return *this;
}

void Object::unsetCons()
{
  if(isA<Cons>())
  {
    assert(isRoot());
    assert(getRefCount() > 0u);
    assert(checkIndex());
    auto coll = static_cast<Cons*>(data.ptr)->getCollector();
    static_cast<Cons*>(data.ptr)->unroot();
    assert(checkIndex());
    coll->step();
    assert(static_cast<Cons*>(data.ptr)->getCollector()->checkSanity());
    assert(checkIndex());
    coll->recycle();
    assert(checkIndex());
  }
  else if(isA<Container>())
  {
    assert(isRoot());
    assert(getRefCount() > 0u);
    assert(checkIndex());
    auto coll = static_cast<Container*>(data.ptr)->getCollector();
    static_cast<Container*>(data.ptr)->unroot();
    coll->step();
    coll->recycle();
  }
}

void Object::init(Cons * cons, TypeId _typeId)
{
  assert(cons->isRoot());
  assert(cons->getRefCount() == 1u);
  Cell::init(cons, _typeId);
}

void Object::init(Container * container, TypeId _typeId)
{
  assert(container->isRoot());
  assert(container->getRefCount() == 1u);
  Cell::init(container, _typeId);
}

Object Lisp::nil(Object::nil());
Object Lisp::undefined(Object::undefined());

