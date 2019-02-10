#pragma once
#include <lpp/core/types/type.h>
#include <memory>

namespace Lisp
{
  class Cons;

  class ConsPattern : public Type<Cons>
  {
  public:
    static std::shared_ptr<ConsPattern> make(std::shared_ptr<BasicType> car=nullptr,
                                             std::shared_ptr<BasicType> cdr=nullptr);
    bool isInstance(const Cell & cell) override;
  private:
    ConsPattern(std::shared_ptr<BasicType> _car,
                std::shared_ptr<BasicType> _cdr);
    std::shared_ptr<BasicType> car;
    std::shared_ptr<BasicType> cdr;
  };
}
