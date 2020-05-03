#include <assert.h>
#include "cell.h"
#include "object.h"
#include "types/cons.h"
#include "types/container.h"
#include <lpp/core/types/array.h> //@todo remove reference when functionality is re-implementated polymorphically
#include <lpp/core/types/symbol.h> //@todo remove reference when functionality is re-implementated polymorphically
#include <lpp/core/types/reference.h> //@todo remove reference when functionality is re-implementated polymorphically
#include <lpp/core/types/function.h> //@todo remove reference when functionality is re-implementated polymorphically

using Cell = Lisp::Cell;
using BasicCons = Lisp::BasicCons;
using Symbol = Lisp::Symbol;
using ManagedType = Lisp::ManagedType;

//@todo use traits to do type dispatch only once
std::string Cell::getTypeName() const
{
  //@todo more types
  if(isA<BasicCons>())
  {
    return "Cons";
  }
  else if(isA<Container>())
  {
    return "Container";
  }
  else
  {
    return "Unknown";
  }
}


void Cell::forEachChild(std::function<void(const Cell&)> func) const
{
  if(isA<BasicCons>())
  {
    as<BasicCons>()->forEachChild(func);
  }
  else if(isA<Container>())
  {
    as<Container>()->forEachChild(func);
  }
}

void Cell::grey() const
{
  auto cons = as<BasicCons>();
  if(cons)
  {
    cons->getContainer()->grey(cons);
    return;
  }
  auto container = as<Container>();
  if(container)
  {
    container->getContainer()->grey(container);
    return;
  }
}


bool Lisp::Cell::isRoot() const
{
  if(isA<BasicCons>())
  {
    //@todo fast as (isA is executed twice)
    as<BasicCons>()->isRoot();
  }
  else if(isA<Container>())
  {
    //@todo fast as
    as<Container>()->isRoot();
  }
  else
  {
    return false;
  }
}

Lisp::Color Lisp::Cell::getColor() const
{
  if(isA<BasicCons>())
  {
    //@todo fast as
    return as<BasicCons>()->getColor();
  }
  else if(isA<Container>())
  {
    //@todo fast as
    return as<Container>()->getColor();
  }
  else
  {
    return Color::Undefined;
  }
}

std::size_t Lisp::Cell::getRefCount() const
{
  if(isA<BasicCons>())
  {
    // @todo fast as
    return as<BasicCons>()->getRefCount();
  }
  else if(isA<Container>())
  {
    // @todo fast as
    return as<Container>()->getRefCount();
  }
  else if(isA<ManagedType>())
  {
    // @todo fast as
    return as<ManagedType>()->getRefCount();
  }
  else
  {
    return 0u;
  }
}

bool Lisp::Cell::checkIndex() const
{
  if(isA<BasicCons>())
  {
    return as<BasicCons>()->checkIndex();
  }
  else if(isA<Container>())
  {
    return as<Container>()->checkIndex();
  }
  else
  {
    true;
  }
}

std::ostream & operator<<(std::ostream & ost, const Lisp::Cell & cell)
{
  using UIntegerType = Lisp::UIntegerType;
  // @todo: implement meta type with introspection and streaming interface
  if(cell.isA<Lisp::Undefined>())
  {
    ost << "[UNDEFINED]";
  }
  else if(cell.isA<Lisp::UIntegerType>())
  {
    ost << cell.as<Lisp::UIntegerType>();
  }
  else if(cell.isA<Lisp::Symbol>())
  {
    ost << cell.as<Lisp::Symbol>()->getName();
  }
  else if(cell.isA<Lisp::Cons>())
  {
    ost << "[CONS " "#" << cell.as<BasicCons>() << " ";
    if(cell.as<Lisp::Cons>()->isRoot())
    {
      ost << " ROOT " << cell.as<Lisp::Cons>()->getRefCount() << " ";
    }
    ost << cell.as<Lisp::Cons>()->getColor()  << "]";
  }
  else if(cell.isA<Lisp::Reference>())
  {
    ost << "[REFERENCE #" << cell.as<Lisp::Reference>() << " ";
    ost << cell.as<Lisp::Reference>()->getCarCell() << "=" << cell.as<Lisp::Reference>()->getValue();
    ost << "]";
  }
  else if(cell.isA<Lisp::Array>())
  {
    ost << "[ARRAY #" << cell.as<Lisp::Array>() << " ";
    if(cell.as<Lisp::Array>()->isRoot())
    {
      ost << " ROOT " << cell.as<Lisp::Array>()->getRefCount() << " ";
    }
    ost << cell.as<Lisp::Array>()->getColor()  << "]";
  }
  else if(cell.isA<Lisp::Function>())
  {
    ost << "[Function " << cell.as<Lisp::Function>() << "]";
  }
  else if(cell.isA<Lisp::Container>())
  {
    ost << "[Container]";
  }
  else if(cell.isA<Lisp::Nil>())
  {
    ost << "[Nil]";
  }
  else
  {
    //@todo more types
    ost << "[BasicType]";
  }
  return ost;
}

bool Cell::operator==(const Lisp::Cell & b) const
{
  if(isA<const ::Lisp::BasicCons>() && b.isA<const ::Lisp::BasicCons>())
  {
    return as<const ::Lisp::BasicCons>() ==  b.as<const ::Lisp::BasicCons>();
  }
  else if(isA<const ::Lisp::ManagedType>() && b.isA<const ::Lisp::ManagedType>())
  {
    return as<const ::Lisp::ManagedType>() == b.as<const ::Lisp::ManagedType>();
  }
  else if(isA<const ::Lisp::Container>() && b.isA<const ::Lisp::Container>())
  {
    return as<const ::Lisp::Container>() == b.as<const ::Lisp::Container>();
  }
  else if(isA<::Lisp::Nil>() && b.isA<::Lisp::Nil>())
  {
    return true;
  }
  else
  {
    // @todo more cases
    return false;
  }
}

std::size_t Cell::hash() const
{
  if(isA<const ::Lisp::BasicCons>())
  {
    static std::hash<const ::Lisp::BasicCons*> hasher;
    return hasher(as<const ::Lisp::BasicCons>());
  }
  else if(isA<const ::Lisp::ManagedType>())
  {
    static std::hash<const ::Lisp::ManagedType*> hasher;
    return hasher(as<const ::Lisp::ManagedType>());
  }
  else if(isA<const ::Lisp::Container>())
  {
    static std::hash<const ::Lisp::Container*> hasher;
    return hasher(as<const ::Lisp::Container>());
  }
  else if(isA<::Lisp::Nil>())
  {
    return 0u;
  }
  else
  {
    /* @todo */
    return 0u;
  }
}
