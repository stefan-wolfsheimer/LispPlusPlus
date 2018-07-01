#include <lpp/core/types/array.h>
using TypeId = Lisp::TypeId;
using Array = Lisp::Array;

void Array::forEachChild(std::function<void(const Cell&)> func) const
{
  for(const Cell & c : data)
  {
    func(c);
  }
}

TypeId Array::getTypeId() const
{
  return TypeTraits<Array>::typeId;
}

bool Array::greyChildren()
{
  if(gcPosition < data.size())
  {
    data[gcPosition].grey();
    if(++gcPosition == data.size())
    {
      gcPosition = 0;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}

void Array::resetGcPosition()
{
  gcPosition = 0;
}

bool Array::recycleNextChild()
{
  if(gcPosition < data.size())
  {
    if(!data[gcPosition].isA<Collectible>())
    {
      data[gcPosition] = Lisp::nil;
    }
    return ++gcPosition == data.size();
  }
  return true;
}
