#pragma once
#include <lpp/core/types/form.h>
#include <lpp/core/types/form_builder.h>

namespace Lisp
{
  template<typename T, typename BUILDER>
  class TypeOf;

  template<typename T>
  class TypeOf<T, void> : public virtual Form
  {
  public:
    TypeOf(std::function<void(const Form * f, const Cell & car)> func=nullptr)
      : cb(func)
    {}

    bool isInstance(const Cell & cell) const override
    {
      return cell.isA<T>();
    }

    bool match(const Cell & cell) const override
    {
      if(cell.isA<T>())
      {
        if(cb)
        {
          cb(this, cell);
        }
        return true;
      }
      else
      {
        return false;
      }
    }
  private:
    std::function<void(const Form * f, const Cell & car)> cb;
  };

  template<typename T, typename BUILDER>
  class TypeOf : public TypeOf<T, void>,
                 public FormBuilder<BUILDER>
  {
    typedef TypeOf<T, void> parent_t;
  public:
    
    TypeOf(void(BUILDER::*cb)(const Cell & cell)=nullptr) : func(cb) {}

    bool match(const Cell & cell, BUILDER & builder) const override
    {
      if(cell.isA<T>())
      {
        if(func)
        {
          ((builder).*func)(cell);
        }
        return true;
      }
      else
      {
        return false;
      }
    }

  private:
    void(BUILDER::*func)(const Cell & cell);
  };

}
