#include <lpp/core/gc/symbol_container.h>
#include <lpp/core/types/symbol.h>
#include <assert.h>

using SymbolContainer = Lisp::SymbolContainer;
using Symbol = Lisp::Symbol;


SymbolContainer::~SymbolContainer()
{
  for(auto & p : symbols)
  {
    assert(p.second->container == this);
    p.second->container = nullptr;
  }
}

Symbol * SymbolContainer::make(const std::string & name)
{
  auto res = symbols.insert(std::make_pair(name, (Symbol*)nullptr));
  if(res.second)
  {
    res.first->second = new Symbol(res.first->first.c_str(), this);
    return res.first->second;
  }
  else
  {
    return res.first->second;
  }
}

Symbol * makeRoot(const std::string & name)
{
}

void SymbolContainer::remove(Symbol * symbol)
{
  auto itr = symbols.find(symbol->getName());
  assert(itr != symbols.end());
  symbols.erase(itr);
}


