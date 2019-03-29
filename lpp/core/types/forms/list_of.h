#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  class ListOf : public Form
  {
  public:
    ListOf(Form * _member, std::function<void(const Cell & car)> func=nullptr);
    bool isInstance(const Cell & cell) const override;
  private:
    Form * member;
    std::function<void(const Cell & car)> cb;
  };
}
