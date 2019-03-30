#include <memory>
#include <lpp/core/default_env.h>
#include <lpp/core/env.h>
#include <lpp/core/memory/allocator.h>
#include <lpp/core/compiler/forms/define.h>
#include <lpp/core/compiler/forms/lambda.h>
#include <lpp/core/types/symbol.h>

using Env = Lisp::Env;
using Compilable = Lisp::Compilable;
using Allocator = Lisp::Allocator;
using Symbol = Lisp::Symbol;

static void defineForm(std::shared_ptr<Env> env,
                       std::shared_ptr<Allocator> alloc,
                       const std::string & name,
                       Compilable * form)
{
  env->set(alloc->makeRoot<Symbol>(name), form);
}

//@todo move to grammar framework (scheme module)
std::shared_ptr<Env> Lisp::makeDefaultEnv(std::shared_ptr<Allocator> alloc)
{
  auto env = std::make_shared<Env>();
  defineForm(env, alloc, "define", alloc->makeRoot<Lisp::Define>());
  defineForm(env, alloc, "lambda", alloc->makeRoot<Lisp::Lambda>());
  return env;
}
