#include <lpp/core/cell.h>
#include <lpp/core/types/container.h>
#include <lpp/core/opcode.h>

namespace Lisp
{
  class Env;

  class ContinuationState
  {
  public:
    inline Function * getFunction() const { return f; }
    ContinuationState(Function * _f, std::size_t _stackFramePos);
  private:
    friend class Continuation;
    using Code = std::vector<InstructionType>;
    using const_iterator = Code::const_iterator;
    Function * f;
    const_iterator itr;
    const_iterator end;
    std::size_t stackFramePos;
  };

  class Continuation : public Container
  {
  public:
    Continuation(const Cell & func, const std::shared_ptr<Env> & _env);
    Continuation(std::vector<Lisp::Cell> && _stack, const std::shared_ptr<Env> & _env);
    inline std::size_t stackSize() const;
    inline void push(const Cell & rhs);
    Cell & eval();

    /* implementation of Container */
    virtual void forEachChild(std::function<void(const Cell&)> func) const override;
    virtual TypeId getTypeId() const override;
    virtual bool greyChildren() override;
    virtual void resetGcPosition() override;
    virtual bool recycleNextChild() override;

  private:
    std::size_t dsPosition;
    std::vector<Lisp::Cell> stack;
    std::vector<Lisp::ContinuationState> callStack;
    std::shared_ptr<Env> env;
  };
}

inline void Lisp::Continuation::push(const Cell & rhs)
{
  rhs.grey();
  stack.push_back(rhs);
}

inline std::size_t Lisp::Continuation::stackSize() const
{
  return stack.size();
}
