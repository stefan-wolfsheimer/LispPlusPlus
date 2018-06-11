#pragma once
//@todo: remove this
namespace Lisp
{
  class GarbageCollectable : public BasicType
  {
    using Color = ...;
    Color getColor();
    size_t getIndex();
    ConsFactory * getConsFactory();
    std::vector<Cell> getChildren();
  };
}
