#pragma once

namespace Lisp
{
  class IConsAllocator
  {
  public:
    virtual ~IConsAllocator() {}
    virtual void * alloc() = 0;
  };
}

