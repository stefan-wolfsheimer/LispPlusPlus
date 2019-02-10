#pragma once
#include <lpp/core/types/basic_type.h>
#include <memory>

namespace Lisp
{
  class ListPattern : public BasicType
  {
  public:
    static std::shared_ptr<ListPattern> make(std::shared_ptr<BasicType> _item=nullptr);
    bool isInstance(const Cell & cell) override;
  private:
    ListPattern(std::shared_ptr<BasicType> _item);
    std::shared_ptr<BasicType> item;
  };
}
