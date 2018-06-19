#pragma once
#include "collectible.h"

namespace Lisp
{
  class Container : public Collectible
  {
  public:
    virtual ~Container(){}
    virtual TypeId getTypeId() = 0;
    inline void unroot();
    inline void root();
    inline void grey();
  protected:
  };
}

inline void Lisp::Container::unroot()
{
  Collectible::unrootInternal<Cons>();
}

inline void Lisp::Container::root()
{
  Collectible::rootInternal<Cons>();
}

inline void Lisp::Container::grey()
{
  Collectible::greyInternal<Cons>();
}
