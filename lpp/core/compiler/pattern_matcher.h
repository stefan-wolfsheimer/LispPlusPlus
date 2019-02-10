#pragma once

namespace Lisp
{
  namespace Pattern
  {
    class BasicIsA
    {
    public:
      virtual ~BasicIsA() {}
      virtual bool isA(const Cell & cell) = 0;
    };

    template<typename TYPE>
    class IsA
    {
    public:
      bool isA(const Cell & cell)
      {
        cell.isA<TYPE>();
      }
    };

    class Or
    {
    };
  }
}
