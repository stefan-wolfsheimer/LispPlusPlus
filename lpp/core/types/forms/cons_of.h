#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  class ConsOf : public Form
  {
  public:
    ConsOf(Form * _car, Form * _cdr, std::function<void(Form *, const Cell & car, Form *, const Cell & cdr)> func=nullptr);
    bool isInstance(const Cell & cell) const override;
    bool match(const Cell & cell) const override;
  private:
    std::function<void(Form *, const Cell & car, Form *, const Cell & cdr)> cb;
    Form * car;
    Form * cdr;
  };
}
