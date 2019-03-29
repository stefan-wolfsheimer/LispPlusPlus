#pragma once
#include <memory>
#include <lpp/core/types/form.h>

/////////////////////////////////////////////////////////////////
namespace Lisp
{
  class GarbageCollector;

  namespace Scheme
  {
    class Language : public ::Lisp::Form
    {
    public:
      Language();
      void init() override;
      bool isInstance(const Cell & cell) const override;
      Object compile(const Cell & cell) const;
    private:
      Form * expression;
    };
  }
}
