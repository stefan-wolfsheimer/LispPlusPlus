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
#include "lisp_object.h"
#include "types/lisp_nil.h"
#include "types/lisp_cons.h"

Lisp::Object::Object(const Object & rhs) : Cell(rhs)
{
  if(rhs.isA<Lisp::Cons>())
  {
    ((Cons*)data.ptr)->root();
  }
}

Lisp::Object::Object(const Cell & rhs) : Cell(rhs)
{
  if(rhs.isA<Lisp::Cons>())
  {
    ((Cons*)data.ptr)->root();
  }
}

Lisp::Object::Object() : Cell(Nil::typeId)
{
}

Lisp::Object::Object(Cons * cons) : Cell(Lisp::Cons::typeId)
{
  assert(cons->isRoot());
  cons->refCount++;
  data.ptr = cons;
}

Lisp::Object::~Object()
{
  unsetCons();
}


Lisp::Object & Lisp::Object::operator=(const Object & rhs)
{
  Cell::unset();
  unsetCons();
  typeId = rhs.typeId;
  if(rhs.isA<Lisp::Cons>())
  {
    assert(rhs.as<Cons>()->isRoot());
    assert(rhs.as<Cons>()->getRefCount() > 0u);
    data.ptr = rhs.as<Cons>();
    ((Cons*)data.ptr)->root();
  }
  return *this;
}

void Lisp::Object::unsetCons()
{
  if(isA<Cons>())
  {
    assert(as<Cons>()->isRoot());
    assert(as<Cons>()->getRefCount() > 0u);
    ((Cons*)data.ptr)->unroot();
  }
}

Lisp::Object Lisp::nil = Lisp::Object();

