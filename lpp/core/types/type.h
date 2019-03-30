#pragma once
#include <lpp/core/types/basic_type.h>
#include <lpp/core/cell.h>



namespace Lisp
{
  //
  // @todo replace by Forms::TypeOf
  template<typename T>
  class Type : public BasicType
  {
  public:
    static std::shared_ptr<Type<T>> make()
    {
      return std::shared_ptr<Type<T>>(new Type());
    }

    bool isInstance(const Cell & cell) override
    {
      return cell.isA<T>();
    }

  protected:
    Type()
    {}
  };

  class AnyType : public BasicType
  {
  public:
    bool isInstance(const Cell & cell) override
    {
      return true;
    }

    static std::shared_ptr<AnyType> make()
    {
      return std::shared_ptr<AnyType>(new AnyType());
    }
  };
}

// Type<Symbol>
// TypeOr<Symbol, Integer>
// TypeOr<Symbol, TypeOr<Integer, Cons> >

