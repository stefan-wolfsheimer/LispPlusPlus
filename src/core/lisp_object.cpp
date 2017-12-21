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
#include "lisp_nil.h"
#include "lisp_cons.h"

Lisp::Object::Object(const Object & rhs) : Cell(rhs.typeId)
{
  if(rhs.isA<Lisp::Cons>())
  {
    data.cons = rhs.data.cons;
    data.cons->root();
  }
}

Lisp::Object::Object(const Cell & rhs) : Cell(rhs.getTypeId())
{
  if(rhs.isA<Lisp::Cons>())
  {
    data.cons = rhs.as<Lisp::Cons>();
    data.cons->root();
  }
}

Lisp::Object::Object() : Cell(Nil::typeId)
{
}

Lisp::Object::Object(Cons * cons) : Cell(Lisp::Cons::typeId)
{
  data.cons = cons;
}

Lisp::Object::~Object()
{
  unsetCons();
}


Lisp::Object & Lisp::Object::operator=(const Object & rhs)
{
  unsetCons();
  typeId = rhs.typeId;
  if(rhs.isA<Lisp::Cons>())
  {
    assert(rhs.data.cons->getColor() == Cons::Color::Root);
    assert(rhs.data.cons->getRefCount() > 0u);
    data.cons = rhs.data.cons;
    data.cons->root();
  }
  return *this;
}

void Lisp::Object::unsetCons()
{
  if(isA<Cons>())
  {
    assert(data.cons->getColor() == Cons::Color::Root);
    assert(data.cons->getRefCount() > 0u);
    data.cons->unroot();
  }
}

Lisp::Object Lisp::nil = Lisp::Object();

