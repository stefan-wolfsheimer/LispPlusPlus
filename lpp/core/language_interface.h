#pragma once
#include <lpp/core/object.h>
namespace Lisp
{
  class LanguageInterface
  {
  public:
    virtual ~LanguageInterface() {}
    virtual Object compile(const Cell & cell) const = 0;
  };
}
