#include "lisp_type_id.h"
#include "lisp_nil.h"
#include "lisp_cons.h"
#include "lisp_symbol.h"
#include "lisp_string.h"

using TypeId = Lisp::TypeId;
const TypeId Lisp::Nil::typeId = 0x0000u;
const TypeId Lisp::Cons::typeId = 0x0001u;
const TypeId Lisp::String::typeId = 0x8001u;
const TypeId Lisp::Symbol::typeId = 0x8002u;
