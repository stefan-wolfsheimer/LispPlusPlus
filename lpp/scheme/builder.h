#pragma once

namespace Lisp
{
  class Cell;
  namespace Scheme
  {
    class Builder
    {
    public:
      void idempotent(const Cell & cell);
      void reference(const Cell & cell);
      void symbol(const Cell & cell);
      void define(const Cell & car, const Cell & cdr);
    };
  }
}
