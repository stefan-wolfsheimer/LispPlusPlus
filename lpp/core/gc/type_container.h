#pragma
#include <lpp/core/types/type.h>
#include <unordered_map>

namespace Lisp
{
  class TypeContainer
  {
  public:
    ~TypeContainer();

    template<typename T>
    BasicType * make();

  private:
    std::unordered_map<TypeId, BasicType*> types;
  };
}

////////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
////////////////////////////////////////////////////////////////////////////////
inline Lisp::TypeContainer::~TypeContainer()
{
  for(auto & p : types)
  {
    delete p.second;
  }
}

template<typename CLS>
Lisp::BasicType * Lisp::TypeContainer::make()
{
  auto itr = types.insert(std::make_pair(TypeTraits<CLS>::getTypeId(), nullptr));
  if(itr.second)
  {
    itr.first->second = new Lisp::Type<CLS>();
  }
  return itr.first->second;
}
