#pragma once
#include <cstdint>

namespace Lisp
{
  class Object
  {
  public:
    friend class Vm;
    /* todo: implement */
    Object(const Object & rhs);
    Object();
    ~Object();

    template<typename T>
    bool isA() const;

  private:
    std::size_t typeId;
  };

  extern Object nil;
}

template<typename T>
bool Lisp::Object::isA() const
{
  return typeId == T::typeId;
}
