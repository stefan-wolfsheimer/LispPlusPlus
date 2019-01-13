#pragma once
#include <memory>
#include <lpp/core/object.h>

namespace Lisp
{
  class GarbageCollector;
  class SymbolContainer;
  class Env;
  class Cell;

  class Jit
  {
  public:
    Jit(std::shared_ptr<GarbageCollector> _gc,
        std::shared_ptr<SymbolContainer> _sc,
        std::shared_ptr<Env> _env);
    void pass1(const Cell & obj);
    void pass2(const Cell & obj);

    std::size_t instrSize;
    std::size_t dataSize;
    Object function;

    std::shared_ptr<GarbageCollector> gc;
    std::shared_ptr<SymbolContainer> sc;
    std::shared_ptr<Env> env;
  };
}
