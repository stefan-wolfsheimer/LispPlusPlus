#pragma once
#include <vector>
#include "lisp_i_cons_allocator.h"

namespace Lisp
{
  class ConsAllocator : public IConsAllocator
  {
  public:
    ~ConsAllocator();
    void * alloc() override;

  private:
    std::vector<void*> allocated;
  };
}

