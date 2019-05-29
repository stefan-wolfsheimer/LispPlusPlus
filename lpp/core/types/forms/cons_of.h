#pragma once
#include <lpp/core/types/form.h>
#include <lpp/core/types/form_builder.h>

namespace Lisp
{
  template<typename BUILDER>
  class ConsOf;

  template<>
  class ConsOf<void> : public virtual Form
  {
  public:
    ConsOf(Form * _car, Form * _cdr, std::function<void(Form *, const Cell & car, Form *, const Cell & cdr)> func=nullptr);
    bool isInstance(const Cell & cell) const override;
    bool match(const Cell & cell) const override;
  protected:
    std::function<void(Form *, const Cell & car, Form *, const Cell & cdr)> cb;
    Form * car;
    Form * cdr;
  };

  template<typename BUILDER>
  class ConsOf : public ConsOf<void>,
                 public FormBuilder<BUILDER>
  {
  public:
    ConsOf(Form * _car,
           Form * _cdr,
           void(BUILDER::*_func)(const Cell & car, const Cell & cdr)=nullptr)
      : func(_func), ConsOf<void>(_car, _cdr)
       {}

    bool match(const Cell & cell, BUILDER & builder) const override
    {
      auto cons = cell.as<Cons>();
      auto carBuilder = dynamic_cast<FormBuilder<BUILDER>*>(car);
      auto cdrBuilder = dynamic_cast<FormBuilder<BUILDER>*>(cdr);
      if (cons &&
          ( carBuilder ?
            carBuilder->match(cell, builder) :
            car->isInstance(cons->getCarCell())) &&
          ( cdrBuilder ?
            cdrBuilder->match(cell, builder) :
            cdr->isInstance(cons->getCarCell())))
      {
        if(func)
        {
          ((builder).*func)(cons->getCarCell(),
                            cons->getCdrCell());
        }
        return true;
      }
      return false;
    }

  private:
    void(BUILDER::*func)(const Cell & car, const Cell & cdr);
  };

}
