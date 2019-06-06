#pragma once
#include <functional>
#include <lpp/core/types/form_builder.h>

namespace Lisp
{
  template<typename BUILDER>
  class ChoiceOf;

  template<>
  class ChoiceOf<void> : public virtual Form
  {
  public:
    ChoiceOf(const std::vector<Form*> & forms=std::vector<Form*>());
    void add(Form * f);
    bool isInstance(const Cell & cell) const override;
  private:
    std::function<void(Form *, const Cell & cell)> cb;
  };

  template<typename BUILDER>
  class ChoiceOf : public ChoiceOf<void>,
                   public FormBuilder<BUILDER>
    
  {
  public:
    ChoiceOf(const std::vector<Form*> & forms=std::vector<Form*>(),
             void(BUILDER::*_func)(const Cell & cell)=nullptr)
      : func(_func)
    {
      for(auto f : forms)
      {
        Form::cells.push_back(f);
      }
    }

    ChoiceOf(void(BUILDER::*_func)(const Cell & cell))
      : func(_func)
    {
    }

    bool match(const Cell & cell, BUILDER & builder) const override
    {
      for(const Cell & c : Form::cells)
      {
        assert(c.isA<Form>());
        auto childBuilder = c.as<FormBuilder<BUILDER>>();
        if(childBuilder)
        {
          if(childBuilder->match(cell, builder))
          {
            if(func)
            {
              ((builder).*func)(cell);
            }
            return true;
          }
        }
        else
        {
          if(c.as<Form>()->isInstance(cell))
          {
            if(func)
            {
              ((builder).*func)(cell);
            }
            return true;
          }
        }
      }
      return false;
    }

  private:
    void(BUILDER::*func)(const Cell & cell);
  };
}
