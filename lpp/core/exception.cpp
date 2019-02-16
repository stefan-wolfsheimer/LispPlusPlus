#include <assert.h>
#include <sstream>
#include <lpp/core/exception.h>
#include <lpp/core/types/function.h>

using Exception = Lisp::Exception;
using ExceptionWithObject = Lisp::ExceptionWithObject;
using NonMatchingArguments = Lisp::NonMatchingArguments;
using NotAList = Lisp::NotAList;
using Object = Lisp::Object;
using Function = Lisp::Function;

const char * Exception::what() const noexcept
{
  return "LispException";
}


ExceptionWithObject::ExceptionWithObject(const Cell & cell) : object(cell)
{
}

const Object & ExceptionWithObject::getObject() const
{
  return object;
}

void ExceptionWithObject::setObject(const Cell & _cell)
{
  object = _cell;
}
/////////////////////////////////////////////////////////////////
NonMatchingArguments::NonMatchingArguments(std::size_t _nargs, Function * f)
  : nargs(_nargs), ExceptionWithObject(Cell(f))
{
  std::stringstream ss;
  ss << "The function [...] has been called with " << nargs
     << ((nargs == 1) ? " argument. " : " arguments. ")
     << "It requires " << f->numArguments()
     << ((f->numArguments() == 1) ? " argument. " : " arguments. ");
  msg = ss.str();
}

const Function * NonMatchingArguments::getFunction() const
{
  assert(getObject().isA<Function>());
  return getObject().as<Function>();
}

std::size_t NonMatchingArguments::getNumArgumentsGiven() const
{
  return nargs;
}

const char * NonMatchingArguments::what() const noexcept
{
  return msg.c_str();
}
