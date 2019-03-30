#pragma once
#include <memory>

namespace Lisp
{
  class SymbolContainer;
  class Allocator;
  class Env;

  std::shared_ptr<Env> makeDefaultEnv(std::shared_ptr<Allocator> _gc,
                                      std::shared_ptr<SymbolContainer> _sc);
}
