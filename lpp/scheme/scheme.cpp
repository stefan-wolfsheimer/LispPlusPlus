#include <lpp/scheme/scheme.h>
#include <lpp/core/cell.h>
#include <lpp/core/gc/garbage_collector.h>
#include <lpp/core/types/forms/choice_of.h>
#include <lpp/core/types/forms/type_of.h>

// lisp core
using Cell = Lisp::Cell;
using Idempotent = Lisp::Idempotent;

// gc
using Guard = Lisp::GarbageCollector::Guard;

// forms 
using ChoiceOf = Lisp::Form::ChoiceOf;
using IdempotentForm = Lisp::Form::TypeOf<Idempotent>;

// scheme specific
using Scheme = Lisp::Scheme::Scheme;


Scheme::Scheme()
{
  expression = nullptr;
}

bool Scheme::isInstance(const Cell & cell) const
{
  return expression->isInstance(cell);
}

void Scheme::init()
{
  Guard _lock(getCollector());
  //expressions 11.4
  expression = make<IdempotentForm>();
  expression = makeRoot<ChoiceOf>(std::vector<Form*>{
      make<IdempotentForm>()
  });
}
