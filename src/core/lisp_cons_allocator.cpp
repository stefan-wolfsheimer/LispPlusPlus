#include "lisp_cons_allocator.h"
#include "lisp_cons.h"

Lisp::ConsAllocator::~ConsAllocator()
{
  for(auto ptr : allocated)
  {
    delete[] (char*)ptr;
  }
}

void * Lisp::ConsAllocator::alloc()
{
  allocated.push_back(new char[sizeof(Cons)]);
  return allocated.back();
}

