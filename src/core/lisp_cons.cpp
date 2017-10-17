#include <assert.h>
#include "lisp_cons.h"
#include "lisp_i_cons_allocator.h"

void * Lisp::Cons::operator new(std::size_t s,
                                IConsAllocator & allocator)
{
  assert(s == sizeof(Lisp::Cons));
  return allocator.alloc();
}

Lisp::Cons::Cons()
{
}

