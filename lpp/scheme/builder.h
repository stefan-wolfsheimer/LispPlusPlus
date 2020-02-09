#pragma once
#include <lpp/core/object.h>

namespace Lisp
{
  class Cell;
  class Allocator;
  class Function;

  namespace Scheme
  {
    class Builder
    {
    public:
      Builder(Allocator * _alloc, Builder * _parent=0);
      void idempotent(const Cell & cell);
      void reference(const Cell & cell);
      void symbol(const Cell & cell);
      void define(const Cell & car, const Cell & cdr);
      void lambdaArgument(const Cell & arg);
      void funcall(const Cell & arg);
      void lambda(const Cell & functionCell);


      void finalize();
      inline const Object & getFunctionObject() const;
      Function * getFunction() const;
    private:
      Builder * parent;
      Function * func;
      Allocator * allocator;
      Object funcObject;
    };
  }
}

const Lisp::Object & Lisp::Scheme::Builder::getFunctionObject() const
{
  return funcObject;
}

inline Lisp::Function * Lisp::Scheme::Builder::getFunction() const
{
  return func;
}
