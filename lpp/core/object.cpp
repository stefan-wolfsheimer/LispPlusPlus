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
#include <lpp/core/memory/allocator.h>

using Nil = Lisp::Nil;
using Object = Lisp::Object;
using BasicCons = Lisp::BasicCons;
using Container = Lisp::Container;

Object::Object(const Object & rhs) : Cell(rhs)
{
  if(rhs.isA<BasicCons>())
  {
    data.pCons->root();
  }
  else if(rhs.isA<Container>())
  {
    data.pContainer->root();
  }
}

Object::Object(const Cell & rhs) : Cell(rhs)
{
  if(rhs.isA<BasicCons>())
  {
    data.pCons->root();
  }
  else if(rhs.isA<Container>())
  {
    data.pContainer->root();
  }
}

Object::~Object()
{
  unsetCons();
}


Object & Object::operator=(const Cell & rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  if(rhs.isA<BasicCons>())
  {
    assert(!rhs.isRoot() || rhs.getRefCount() > 0u);
    assert(rhs.checkIndex());
    data.pCons = rhs.data.pCons;
    data.pCons->root();
  }
  else if(rhs.isA<Container>())
  {
    assert(!rhs.isRoot() || rhs.getRefCount() > 0u);
    assert(rhs.checkIndex());
    data.pContainer = rhs.data.pContainer;
    data.pContainer->root();
  }
  else if(rhs.isA<ManagedType>())
  {
    Cell::init(rhs.as<ManagedType>(), rhs.getTypeId());
  }
  else
  {
    data = rhs.data;
  }
  return *this;
}

Object & Object::operator=(const Object & rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  if(rhs.isA<BasicCons>())
  {
    assert(rhs.isRoot());
    assert(rhs.getRefCount() > 0u);
    assert(rhs.checkIndex());
    data.pCons = rhs.data.pCons;
    data.pCons->incRefCount();
  }
  else if(rhs.isA<Lisp::Container>())
  {
    assert(rhs.isRoot());
    assert(rhs.getRefCount() > 0u);
    assert(rhs.checkIndex());
    data.pContainer = rhs.data.pContainer;
    data.pContainer->incRefCount();
  }
  else if(rhs.isA<Lisp::ManagedType>())
  {
    Cell::init(rhs.as<ManagedType>(), rhs.getTypeId());
  }
  else
  {
    data = rhs.data;
  }
  return *this;
}


Object & Object::operator=(Cell && rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  data = rhs.data;
  rhs.typeId = TypeTraits<Nil>::getTypeId();
  return *this;
}

Object & Object::operator=(Object && rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  data = rhs.data;
  rhs.typeId = TypeTraits<Nil>::getTypeId();
  return *this;
}

void Object::unsetCons()
{
  if(isA<BasicCons>())
  {
    assert(isRoot());
    assert(getRefCount() > 0u);
    assert(checkIndex());
    auto coll = data.pCons->getAllocator();
    data.pCons->unroot();
    assert(checkIndex());
    coll->step();
    assert(data.pCons->getAllocator()->checkSanity());
    assert(checkIndex());
    coll->recycle();
    assert(checkIndex());

  }
  else if(isA<Container>())
  {
    assert(isRoot());
    assert(getRefCount() > 0u);
    assert(checkIndex());
    auto coll = data.pContainer->getAllocator();
    data.pContainer->unroot();
    coll->step();
    coll->recycle();
  }
}

void Object::init(BasicCons * cons, TypeId _typeId)
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

void Object::swap(Object & rhs)
{
  TypeId tmpId = rhs.typeId;
  rhs.typeId = typeId;
  typeId = tmpId;
  CellDataType tmpData = rhs.data;
  rhs.data = data;
  data = tmpData;
}

Object Lisp::nil(Object::nil());
Object Lisp::undefined(Object::undefined());

