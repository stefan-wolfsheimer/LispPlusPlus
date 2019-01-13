#include <memory>
#include <lpp/core/default_env.h>
#include <lpp/core/env.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/gc/garbage_collector.h>

using Env = Lisp::Env;


std::shared_ptr<Env> Lisp::makeDefaultEnv(std::shared_ptr<GarbageCollector> _gc,
                                          std::shared_ptr<SymbolContainer> _sc)
{
  auto ret = std::make_shared<Env>();
  return ret;
}
