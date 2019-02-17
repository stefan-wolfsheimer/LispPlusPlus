#include <assert.h>
#include "cell.h"
#include "object.h"
#include "types/cons.h"
#include "types/container.h"
#include <lpp/core/types/array.h> //@todo remove reference when functionality is re-implementated polymorphically
#include <lpp/core/types/symbol.h> //@todo remove reference when functionality is re-implementated polymorphically
#include <lpp/core/types/reference.h> //@todo remove reference when functionality is re-implementated polymorphically

using Cell = Lisp::Cell;
using BasicCons = Lisp::BasicCons;
using Symbol = Lisp::Symbol;
using ManagedType = Lisp::ManagedType;

std::string Cell::getTypeName() const
{
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
    cons->grey();
    return;
  }
  auto container = as<Container>();
  if(container)
  {
    container->grey();
    return;
  }
}


bool Lisp::Cell::isRoot() const
{
  if(isA<BasicCons>())
  {
    as<BasicCons>()->isRoot();
  }
  else if(isA<Container>())
  {
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
    return as<BasicCons>()->getColor();
  }
  else if(isA<Container>())
  {
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
    return as<BasicCons>()->getRefCount();
  }
  else if(isA<Container>())
  {
    return as<Container>()->getRefCount();
  }
  else if(isA<ManagedType>())
  {
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
  using IntegerType = Lisp::IntegerType;
  // @todo: implement meta type with introspection and streaming interface
  if(cell.isA<Lisp::Undefined>())
  {
    ost << "[UNDEFINED]";
  }
  else if(cell.isA<Lisp::IntegerType>())
  {
    ost << cell.as<Lisp::IntegerType>();
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
    ost << "[Function]";
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
