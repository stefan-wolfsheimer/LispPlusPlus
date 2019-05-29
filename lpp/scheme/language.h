#pragma once
#include <memory>
#include <lpp/core/types/form.h>
#include <lpp/core/types/form_builder.h>
#include <lpp/core/language_interface.h>
#include <lpp/scheme/builder.h>

/////////////////////////////////////////////////////////////////
namespace Lisp
{
  class Allocator;
  class Vm;

  template<typename B>
  class ChoiceOf;

  namespace Scheme
  {
    class Language : public ::Lisp::Form,
                     public ::Lisp::LanguageInterface
    {
    public:
      Language();
      void init() override;
      bool isInstance(const Cell & cell) const override;
      Object compile(const Cell & cell) const override;
      bool match(const Cell & cell) const override;
    private:
      ChoiceOf<void> * expression;
      FormBuilder<Builder> * topLevelForm;
      Form * lambdaForm;
    };
  }
}
