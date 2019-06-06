#pragma once
#include <lpp/core/types/form.h>
#include <lpp/core/types/form_builder.h>

namespace Lisp
{
  template<typename BUILDER>
  class ListOf;

  template<>
  class ListOf<void> : public virtual Form
  {
  public:
    ListOf(Form * _member);
    bool isInstance(const Cell & cell) const override;
  protected:
    Form * member;
  };


  template<typename BUILDER>
  class ListOf : public ListOf<void>,
                 public FormBuilder<BUILDER>
  {
  public:
    ListOf(Form * _member,
           void(BUILDER::*_func)(const Cell & car)=nullptr,
           void(BUILDER::*_eolfunc)(const Cell & lst)=nullptr)
      : func(_func), eolfunc(_eolfunc), ListOf<void>(_member)
       {
       }

    bool match(const Cell & cell, BUILDER & builder) const override
    {
      const Cell * c = &cell;
      auto memberBuilder = dynamic_cast<FormBuilder<BUILDER>*>(member);
      while(true)
      {
        if(c->isA<Nil>())
        {
          if(eolfunc)
          {
            ((builder).*eolfunc)(cell);
          }
          return true;
        }
        Cons * cons = c->as<Cons>();
        if(cons &&
           (memberBuilder ?
            memberBuilder->match(cons->getCarCell(), builder) :
            member->isInstance(cons->getCarCell())))
        {
          if(func)
          {
            ((builder).*func)(cons->getCarCell());
          }
          c = &cons->getCdrCell();
        }
        else
        {
          return false;
        }
      }
      return false;
    }

  private:
    void(BUILDER::*func)(const Cell & member);
    void(BUILDER::*eolfunc)(const Cell & member);
  };

}
