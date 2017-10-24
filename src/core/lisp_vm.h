#pragma once
#include <memory>

namespace Lisp
{
  class IConsAllocator;
  class ConsAllocator;
  class Vm
  {
  public:
    Vm(std::shared_ptr<IConsAllocator> alloc=nullptr);
  private:
    std::shared_ptr<IConsAllocator> consAllocator;
  };
}
