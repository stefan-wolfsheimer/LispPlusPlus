#include "lisp_object.h"
#include "lisp_nil.h"

Lisp::Object::Object() : typeId(Nil::typeId)
{
}

Lisp::Object::~Object()
{
}

Lisp::Object Lisp::nil = Lisp::Object();
