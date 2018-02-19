#include "lisp_nil.h"
#include "lisp_cons.h"
#include "lisp_symbol.h"

const std::size_t Lisp::Nil::typeId = 0u;
const std::size_t Lisp::Cons::typeId = 1u;
const std::size_t Lisp::Symbol::typeId = 2u;
