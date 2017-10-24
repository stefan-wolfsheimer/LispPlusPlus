#include "lisp_vm.h"
#include "lisp_cons_allocator.h"

Lisp::Vm::Vm(std::shared_ptr<IConsAllocator> allocator)
  : consAllocator(allocator ? allocator : std::make_shared<ConsAllocator>())
{
}
