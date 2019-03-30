#pragma once
#include <memory>

namespace Lisp
{
  class Allocator;
  class Env;

  std::shared_ptr<Env> makeDefaultEnv(std::shared_ptr<Allocator> _alloc);
}
