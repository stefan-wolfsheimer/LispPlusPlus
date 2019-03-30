#pragma once
#include <assert.h>
#include <functional>
#include <lpp/core/config.h>
#include <lpp/core/types/type_id.h>
#include <lpp/core/memory/color.h>
#include <lpp/core/memory/collectible_container.h>

namespace Lisp
{
  class Allocator;

  class Collectible
  {
  };

  /**
   * Every class that has sub garbage collected sub-cells
   * should be derived from this mixin class.
   *
   */
  template<typename T>
  class CollectibleMixin
  {
  public:
    /* friendship for setting the index in the CollectibleContainer
     */
    friend class CollectibleContainer<T>;

    /* friendship for setting the reference count
     */
    friend class Allocator;

    /* friendship for increasing the reference count
     */
    friend class Object;

    CollectibleMixin();

    /**
     * Nuber of references to this object.
     * The number of Object instances that box this object.
     * Cell instances are not counted.
     */
    inline std::size_t getRefCount() const;

    inline Color getColor() const;
    inline bool isRoot() const;
    inline std::size_t getIndex() const;
    inline Allocator * getAllocator() const;
    inline CollectibleContainer<T> * getContainer() const;
    inline bool checkIndex();

    /**
     * If collectible is white, move it to grey.
     * Otherwise do nothing.
     */
    inline void grey();
  private:

    inline void setRefCount(std::size_t);
    inline void incRefCount();
    inline void unroot();
    inline void root();
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
inline Lisp::Allocator * Lisp::CollectibleMixin<T>::getAllocator() const
{
  return container->getAllocator();
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
    // only if container is white
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
