#pragma once
#include <lpp/core/types/polymorphic_container.h>
#include <lpp/core/gc/garbage_collector.h>

namespace Lisp
{
  class Jit;

  class Form : public PolymorphicContainer
  {
  public:
    /**
     * Create a temporary form that can be used
     * as subform.
     */
    template<typename T,  typename... ARGS>
    T * make(ARGS ...rest);

    /**
     * Create a form
     */
    template<typename T,  typename... ARGS>
    T * makeRoot(ARGS ...rest);

    virtual bool isInstance(const Cell & cell) const = 0;
    virtual void forEachChild(std::function<void(const Cell&)> func) const override;
    virtual bool greyChildren() override;
    virtual void resetGcPosition() override;
    virtual bool recycleNextChild() override;

  protected:
    std::vector<Cell> cells;
  private:
    template<typename T,  typename... ARGS>
    T * _make(std::true_type, ARGS ...rest);

    template<typename T,  typename... ARGS>
    T * _makeRoot(std::true_type, ARGS ...rest);
  };

  //@todo move to grammar
  class Compilable : public Form
  {
  public:
    virtual void compile(Jit & jit, Function *, const Cell & obj) const=0;
  };
}

template<typename T,  typename... ARGS>
T * Lisp::Form::Form::make(ARGS ...rest)
{
  return _make<T>(typename std::is_base_of<Form, T>::type(),
                  rest...);
}

template<typename T,  typename... ARGS>
T * Lisp::Form::Form::makeRoot(ARGS ...rest)
{
  return _makeRoot<T>(typename std::is_base_of<Form, T>::type(),
                      rest...);
}

template<typename T,  typename... ARGS>
T * Lisp::Form::Form::_make(std::true_type, ARGS ...rest)
{
  return getCollector()->make<T>(rest...);
}

template<typename T,  typename... ARGS>
T * Lisp::Form::Form::_makeRoot(std::true_type, ARGS ...rest)
{
  T * form = getCollector()->make<T>(rest...);
  cells.push_back(form);
  return form;
}
