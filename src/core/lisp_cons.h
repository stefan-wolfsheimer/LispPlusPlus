#pragma once
#include <cstdint>

namespace Lisp
{
  class IConsAllocator;
  class Cons
  {
  public:
    static const std::size_t typeId;
    void * operator new(std::size_t s,
                        IConsAllocator & allocator);
    Cons();
  };
}
