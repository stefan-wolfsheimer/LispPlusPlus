#pragma once
#include <vector>
#include <lpp/core/object.h>
#include <lpp/core/types/function.h>

namespace Lisp
{
  class Allocator;
  class Cell;
  class Form;

  namespace Scheme
  {
    class Context
    {
    public:
      Context(Allocator * _alloc);
      ~Context();

      static void idempotentForm(const Form * form, const Cell & cell);
      static void symbolForm(const Form * form, const Cell & cell);
      static void defineForm(Form *, const Cell & car, Form *, const Cell & cdr);
      static void argumentListForm(const Cell & car);

      static void functionEvaluationArgumentForm();
      static void functionEvaluationForm(std::size_t n, const Cell & func);

      static inline std::vector<Context*>& getContextStack();
      inline void finalize();
      inline Function * getFunction();
      inline const Object & getFunctionObject() const;
    private:
      Allocator * alloc;
      Object funcObject;
      Function * f;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// Implmentation
//
///////////////////////////////////////////////////////////////////////////////
inline Lisp::Scheme::Context::~Context()
{
  getContextStack().pop_back();
}

inline std::vector<Lisp::Scheme::Context*>& Lisp::Scheme::Context::getContextStack()
{
  static std::vector<Context*> ctxStack;
  return ctxStack;
}

inline void Lisp::Scheme::Context::finalize()
{
  f->shrink();
}

inline Lisp::Function * Lisp::Scheme::Context::getFunction()
{
  return f;
}

inline const Lisp::Object & Lisp::Scheme::Context::getFunctionObject() const
{
  return funcObject;
}
