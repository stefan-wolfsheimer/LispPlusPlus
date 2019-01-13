#include <memory>
#include <lpp/core/default_env.h>
#include <lpp/core/env.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/forms/define.h>

using Env = Lisp::Env;
using SymbolContainer = Lisp::SymbolContainer;
using Form = Lisp::Form;

static void defineForm(std::shared_ptr<Env> env,
                       std::shared_ptr<SymbolContainer> sc,
                       const std::string & name,
                       Form * form)
{
  env->set(sc->make(name), form);
}

std::shared_ptr<Env> Lisp::makeDefaultEnv(std::shared_ptr<GarbageCollector> gc,
                                          std::shared_ptr<SymbolContainer> sc)
{
  auto env = std::make_shared<Env>();
  defineForm(env, sc, "define", new Lisp::Define());
  return env;
}
