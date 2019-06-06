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
    ConsOf(Form * _car, Form * _cdr);
    bool isInstance(const Cell & cell) const override;
  protected:
    Form * car;
    Form * cdr;
  };

  template<typename BUILDER>
  class ConsOf : public ConsOf<void>,
                 public FormBuilder<BUILDER>
  {
  public:
    typedef ConsOf<BUILDER> self_t;

    ConsOf(Form * _car, Form * _cdr,
           void(BUILDER::*_func)(const Cell & car, const Cell & cdr)=nullptr)
      : matchFunc(_func),
        ConsOf<void>(_car, _cdr)
       {}

    bool match(const Cell & cell, BUILDER & builder) const override
    {
      auto cons = cell.as<Cons>();
      auto carBuilder = dynamic_cast<FormBuilder<BUILDER>*>(car);
      auto cdrBuilder = dynamic_cast<FormBuilder<BUILDER>*>(cdr);
      if (cons &&
          ( carBuilder ?
            carBuilder->match(cons->getCarCell(), builder) :
            car->isInstance(cons->getCarCell())) &&
          ( cdrBuilder ?
            cdrBuilder->match(cons->getCdrCell(), builder) :
            cdr->isInstance(cons->getCdrCell())))
      {
        if(matchFunc)
        {
          ((builder).*matchFunc)(cons->getCarCell(),
                                 cons->getCdrCell());
        }
        return true;
      }
      return false;
    }
  private:
    void(BUILDER::*matchFunc)(const Cell & car, const Cell & cdr);
    void(BUILDER::*beforeFunc)(const Cell & cell);
  };

}
