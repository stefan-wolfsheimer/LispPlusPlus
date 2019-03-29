#include <memory>
#include <lpp/core/default_env.h>
#include <lpp/core/env.h>
#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/compiler/forms/define.h>
#include <lpp/core/compiler/forms/lambda.h>

using Env = Lisp::Env;
using SymbolContainer = Lisp::SymbolContainer;
using Compilable = Lisp::Compilable;

static void defineForm(std::shared_ptr<Env> env,
                       std::shared_ptr<SymbolContainer> sc,
                       const std::string & name,
                       Compilable * form)
{
  env->set(sc->make(name), form);
}

//@todo move to grammar framework (scheme module)
std::shared_ptr<Env> Lisp::makeDefaultEnv(std::shared_ptr<GarbageCollector> gc,
                                          std::shared_ptr<SymbolContainer> sc)
{
  auto env = std::make_shared<Env>();
  defineForm(env, sc, "define", gc->makeRoot<Lisp::Define>());
  defineForm(env, sc, "lambda", gc->makeRoot<Lisp::Lambda>());
  return env;
}
