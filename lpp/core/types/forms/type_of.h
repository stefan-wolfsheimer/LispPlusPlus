#pragma once
#include <lpp/core/types/form.h>

namespace Lisp
{
  namespace Form
  {
    template<typename T>
    class TypeOf : public Form::Form
    {
    public:
      bool isInstance(const Cell & cell) const override
      {
        return cell.isA<T>();
      }

    };
  }
}
