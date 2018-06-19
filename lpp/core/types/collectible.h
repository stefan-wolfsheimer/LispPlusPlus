#pragma once
#include <assert.h>
#include <functional>
#include <lpp/core/types/type_id.h>
#include <lpp/core/gc/color.h>
#include <lpp/core/gc/basic_collectible_container.h>

namespace Lisp
{
  class GarbageCollector;

  class Collectible : public BasicType
  {
  public:
    template<typename T>
    friend class CollectibleContainer;

    Collectible();
    using Color = Lisp::Color;
    inline std::size_t getRefCount() const;
    inline void setRefCount(std::size_t);
    inline void incRefCount();
    inline Color getColor() const;
    inline bool isRoot() const;
    inline std::size_t getIndex() const;
    
    template<typename T>
    inline CollectibleContainer<T> * getContainer() const;

  protected:
    template<typename T>
    inline void unrootInternal();

    template<typename T>
    inline void rootInternal();

    template<typename T>
    inline void greyInternal();

  private:
    std::size_t refCount;
    CollectibleContainer<void> * container;
    std::size_t index;
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
inline Lisp::Collectible::Collectible()
{
  refCount = 0;
}

std::size_t Lisp::Collectible::getRefCount() const
{
  return refCount;
}

inline void Lisp::Collectible::setRefCount(std::size_t rc)
{
  refCount = rc;
}

inline void Lisp::Collectible::incRefCount()
{
  assert(isRoot());
  refCount++;
}



Lisp::Color Lisp::Collectible::getColor() const
{
  return container->getColor();
}

bool Lisp::Collectible::isRoot() const
{
  return container->isRoot();
}

std::size_t Lisp::Collectible::getIndex() const
{
  return index;
}

template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::Collectible::getContainer() const
{
  return static_cast<Lisp::CollectibleContainer<T>*>(container);
}

template<typename T>
inline void Lisp::Collectible::unrootInternal()
{
  if(!--refCount)
  {
    static_cast<Lisp::CollectibleContainer<T>*>(container)->unroot(static_cast<T*>(this));
  }
}

template<typename T>
inline void Lisp::Collectible::rootInternal()
{
  if(isRoot())
  {
    ++refCount;
  }
  else
  {
    static_cast<Lisp::CollectibleContainer<T>*>(container)->root(static_cast<T*>(this));
  }
}

template<typename T>
inline void Lisp::Collectible::greyInternal()
{
  auto greyContainer = container->getGreyContainer<T>();
  if(greyContainer)
  {
    static_cast<Lisp::CollectibleContainer<T>*>(container)->remove(static_cast<T*>(this));
    greyContainer->add(static_cast<T*>(this));
  }
}
