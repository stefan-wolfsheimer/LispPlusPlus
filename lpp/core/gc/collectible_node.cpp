#include "collectible_node.h"
#include <lpp/core/gc/collectible_edge.h>

using Cell = Lisp::Cell;
using CollectibleNode = Lisp::CollectibleNode;

std::vector<std::pair<Cell, std::size_t>> CollectibleNode::getParents() const
{
  std::vector<std::pair<Cell, std::size_t>> ret;
  for(auto p : parents)
  {
    if(!p->getParent().isA<Nil>())
    {
      ret.push_back(std::make_pair(p->getParent(), p->getIndex()));
    }
  }
  return ret;
}

std::vector<std::pair<Cell, std::size_t>> CollectibleNode::getChildren() const
{
  std::vector<std::pair<Cell, std::size_t>> ret;
  for(auto p : children)
  {
    if(!p->getChild().isA<Nil>())
    {
      ret.push_back(std::make_pair(p->getChild(), p->getIndex()));
    }
  }
  return ret;
}
