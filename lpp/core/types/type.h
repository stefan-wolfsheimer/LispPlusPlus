#pragma once
#include <lpp/core/types/basic_type.h>
#include <lpp/core/cell.h>


namespace Lisp
{
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

    // @todo remove static approach for defining forms
    static bool isInstanceStatic(const Cell & cell)
    {
      return cell.isA<T>();
    }

  protected:
    friend class TypeContainer;

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

  /**
   * Matches form against one of two types.
   * @todo remove static version and build
   *       virtual form definitions
   */
  template<typename T1, typename T2>
  class ChoiceType : public BasicType
  {
  public:
    /**
     * @return true if cell is an instance of T1 or T2
     */
    bool isInstance(const Cell & cell) override
    {
      return isInstanceStatic(cell);
    }

    static bool isInstanceStatic(const Cell & cell)
    {
      return T1::isInstanceStatic(cell) || T2::isInstanceStatic(cell);
    }
  };
}

// Type<Symbol>
// TypeOr<Symbol, Integer>
// TypeOr<Symbol, TypeOr<Integer, Cons> >

