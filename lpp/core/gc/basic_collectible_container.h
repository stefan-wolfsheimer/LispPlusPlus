#pragma once
#include <lpp/core/gc/color.h>

namespace Lisp
{
  class GarbageCollector;
  
  template<typename T>
  class CollectibleContainer;

  template<>
  class CollectibleContainer<void>
  {
  public:
    CollectibleContainer(Color _color,
                         bool _isRoot,
                         GarbageCollector * _gc);
    inline Color getColor() const;
    inline GarbageCollector * getCollector() const;
    inline bool isRoot() const;
    inline bool isGrey() const;

    template<typename T>
    CollectibleContainer<T> * getOtherContainer() const;

    template<typename T>
    CollectibleContainer<T> * getGreyContainer() const;

    template<typename T>
    CollectibleContainer<T> * getToContainer() const;

    void collect();
  private:
    template<typename X>
    friend class ColorMap;

    friend class GarbageCollector; //@tood remove this friendship
    // if color is non root then otherElements is fromRootColor
    CollectibleContainer<void> * otherElements;
    CollectibleContainer<void> * greyElements;
    CollectibleContainer<void> * toElements;
    GarbageCollector * gc;
    Color color;
    bool _isRoot;
  };
}

template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleContainer<void>::getOtherContainer() const
{
  return static_cast<Lisp::CollectibleContainer<T> *>(otherElements);
}

template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleContainer<void>::getGreyContainer() const
{
  return static_cast<Lisp::CollectibleContainer<T> *>(greyElements);
}


template<typename T>
inline Lisp::CollectibleContainer<T> * Lisp::CollectibleContainer<void>::getToContainer() const
{
  return static_cast<Lisp::CollectibleContainer<T> *>(toElements);
}

inline Lisp::CollectibleContainer<void>::CollectibleContainer(Color _color,
                                                              bool __isRoot,
                                                              GarbageCollector * _gc)
  : color(_color), gc(_gc), _isRoot(__isRoot)
{
}


inline Lisp::Color Lisp::CollectibleContainer<void>::getColor() const
{
  return color;
}


inline Lisp::GarbageCollector * Lisp::CollectibleContainer<void>::getCollector() const
{
  return gc;
}


inline bool Lisp::CollectibleContainer<void>::isRoot() const
{
  return _isRoot;
}

inline bool Lisp::CollectibleContainer<void>::isGrey() const
{
  return color == Color::Grey;
}


