#pragma once
#include <exception>
#include <lpp/core/object.h>
#include <lpp/core/exception.h>

namespace Lisp
{
  class Function;

  class Exception : public std::exception
  {
  public:
    virtual const char * what() const noexcept override;
  };

  class ExceptionWithObject : public Exception
  {
  public:
    ExceptionWithObject(const Cell & _cell);
    const Object & getObject() const;
    void setObject(const Cell & _cell);
  private:
    Object object;
  };

  class NotAList : public ExceptionWithObject
  {
  public:
    NotAList(const Cell & _cell) : ExceptionWithObject(_cell) {};

    virtual const char * what() const noexcept override
    {
      return "NotAList";
    }
  };

  class IllFormed : public ExceptionWithObject
  {
  public:
    IllFormed(const Cell & _cell) : ExceptionWithObject(_cell) {};

    virtual const char * what() const noexcept override
    {
      return "IllFormed";
    }
  };

  /**
   * A function argument symbol is given more than one time.
   */
  class ArgumentGivenTwice : public IllFormed
  {
  public:
    ArgumentGivenTwice(const Cell & _symbol) : IllFormed(Lisp::nil)
    {
    }

    ArgumentGivenTwice(const Cell & _symbol,
                       const Cell & _list) : IllFormed(_list)
    {
    }

    const Object & getSymbol() const
    {
      return symbol;
    }

  private:
    Object symbol;
  };

  class NonMatchingArguments : public ExceptionWithObject
  {
  public:
    NonMatchingArguments(std::size_t nargs, Function * f);
    const Function * getFunction() const;
    std::size_t getNumArgumentsGiven() const;
    virtual const char * what() const noexcept override;
  private:
    std::string msg;
    std::size_t nargs;
  };
}
