#pragma once
#include <assert.h>
#include <functional>
#include <lpp/core/types/type_id.h>
#include <lpp/core/gc/color.h>
#include <lpp/core/gc/collectible_container.h>

namespace Lisp
{
  class GarbageCollector;

  class Collectible : public BasicType
  {
  };

  template<typename T>
  class CollectibleMixin
  {
  public:
    friend class CollectibleContainer<T>;

    CollectibleMixin();
    inline std::size_t getRefCount() const;
    inline void setRefCount(std::size_t);
    inline void incRefCount();
    inline Color getColor() const;
    inline bool isRoot() const;
    inline std::size_t getIndex() const;
    inline GarbageCollector * getCollector() const;
    inline CollectibleContainer<T> * getContainer() const;
    inline void unroot();
    inline void root();
    inline void grey();
    inline bool checkIndex();
  private:
    CollectibleContainer<T> * container;
    std::size_t refCount;
    std::size_t index;
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
template<typename T>
inline Lisp::CollectibleMixin<T>::CollectibleMixin()
{
  refCount = 0;
}

template<typename T>
inline std::size_t Lisp::CollectibleMixin<T>::getRefCount() const
{
  return refCount;
}

template<typename T>
inline void Lisp::CollectibleMixin<T>::setRefCount(std::size_t rc)
{
  refCount = rc;
}

template<typename T>
inline void Lisp::CollectibleMixin<T>::incRefCount()
{
  assert(isRoot());
  refCount++;
}

template<typename T>
Lisp::Color Lisp::CollectibleMixin<T>::getColor() const
{
  return container->getColor();
}

template<typename T>
bool Lisp::CollectibleMixin<T>::isRoot() const
{
  return container->isRoot();
}

template<typename T>
std::size_t Lisp::CollectibleMixin<T>::getIndex() const
{
  return index;
}

template<typename T>
inline Lisp::GarbageCollector * Lisp::CollectibleMixin<T>::getCollector() const
{
  return container->getCollector();
}

template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleMixin<T>::getContainer() const
{
  return container;
}

template<typename T>
inline void Lisp::CollectibleMixin<T>::unroot()
{
  if(!--refCount)
  {
    container->unroot(static_cast<T*>(this));
  }
}

template<typename T>
inline void Lisp::CollectibleMixin<T>::root()
{
  if(isRoot())
  {
    ++refCount;
  }
  else
  {
    container->root(static_cast<T*>(this));
  }
}

template<typename T>
inline void Lisp::CollectibleMixin<T>::grey()
{
  auto greyContainer = container->getGreyContainer();
  if(greyContainer)
  {
    container->remove(static_cast<T*>(this));
    greyContainer->add(static_cast<T*>(this));
  }
}

template<typename T>
inline bool Lisp::CollectibleMixin<T>::checkIndex()
{
  if(index >= container->size())
  {
    return false;
  }
  return (*container)[index] == this;
}
