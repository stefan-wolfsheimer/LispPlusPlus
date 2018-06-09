#pragma once
#include <functional>
#include <lpp/core/types/type_id.h>
#include <lpp/core/gc/color.h>

namespace Lisp
{
  class Collectible : public BasicType
  {
  public:
    using Color = Lisp::Color;
    inline std::size_t getRefCount() const;
    inline Color getColor() const;
    inline bool isRoot() const;
    inline std::size_t getIndex() const;
  protected:
    // todo reduce memory footprint
    // reference to color vector
    ConsFactory * consFactory;
    Color color;
    std::size_t refCount;
    std::size_t index;
  };
}

/******************************************************************************
 * Implementation
 ******************************************************************************/
std::size_t Lisp::Collectible::getRefCount() const
{
  return refCount;
}

Lisp::Color Lisp::Collectible::getColor() const
{
  return color;
}

bool Lisp::Collectible::isRoot() const
{
  return
    color == Color::WhiteRoot ||
    color == Color::BlackRoot ||
    color == Color::GreyRoot;
}

std::size_t Lisp::Collectible::getIndex() const
{
  return index;
}
