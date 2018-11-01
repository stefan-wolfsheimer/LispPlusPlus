#pragma once
#include <iostream>

namespace Lisp
{
  class RunStatistics
  {
  public:
    virtual ~RunStatistics() {}
    virtual std::size_t numSteps() const = 0;
    virtual std::ostream & streamHeader(std::ostream & ost) const = 0;
    virtual std::ostream & streamRow(std::ostream & ost, std::size_t i) const = 0;
  };
}
