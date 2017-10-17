#pragma once

namespace Lisp
{
  class Cons
  {
  public:
    void * operator new(std::size_t s,
                        IConsAllocator & allocator);
    Cons();
  };
}

