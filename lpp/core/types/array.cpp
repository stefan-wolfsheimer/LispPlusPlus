#include <lpp/core/types/array.h>
using TypeId = Lisp::TypeId;
using Array = Lisp::Array;

TypeId Array::getTypeId() const
{
  return TypeTraits<Array>::typeId;
}
