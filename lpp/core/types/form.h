#pragma once
#include <lpp/core/types/polymorphic_container.h>


namespace Lisp
{
  // @todo refactor XForm -> Form
  class Jit;

  namespace Form
  {
    class Form : public PolymorphicContainer
    {
    public:
      virtual bool isInstance(const Cell & cell) const = 0;
      virtual void forEachChild(std::function<void(const Cell&)> func) const override;
      virtual bool greyChildren() override;
      virtual void resetGcPosition() override;
      virtual bool recycleNextChild() override;

    protected:
      std::vector<Cell> cells;
    };

    //@todo move to grammar
    class Compilable : public Form
    {
    public:
      virtual void compile(Jit & jit, Function *, const Cell & obj) const=0;
    };
  }
}
