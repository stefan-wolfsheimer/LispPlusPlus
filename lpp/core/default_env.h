#pragma once
#include <memory>

namespace Lisp
{
  class SymbolContainer;
  class GarbageCollector;
  class Env;

  std::shared_ptr<Env> makeDefaultEnv(std::shared_ptr<GarbageCollector> _gc,
                                      std::shared_ptr<SymbolContainer> _sc);
}
