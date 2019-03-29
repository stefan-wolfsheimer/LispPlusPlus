#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  template<typename T>
  class TypeOf : public Form::Form
  {
  public:
    TypeOf(std::function<void(const Form * f, const Cell & car)> func=nullptr)
      : cb(func)
    {}

    bool isInstance(const Cell & cell) const override
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
}
