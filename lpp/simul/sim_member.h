/******************************************************************************
Copyright (c) 2018, Stefan Wolfsheimer

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
******************************************************************************/
#pragma once
#include <string>
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <typeindex>
#include <type_traits>


namespace Lisp
{
  template<typename CLS>
  class SimulMemberBase
  {
  public:
    SimulMemberBase(const std::string & _name);
    virtual ~SimulMemberBase() {}
    inline const std::string& getName() const;
    virtual void write(std::ostream & ost, const CLS * obj) const = 0;
    virtual void read(std::istream & ist, CLS * obj) const = 0;
    virtual void sort(std::vector<CLS> & data) const = 0;
    virtual double sum(const std::vector<CLS> & data) const = 0;
    virtual double sum2(const std::vector<CLS> & data, double offset=0) const = 0;
    virtual void copy(CLS * a, const CLS * b) const = 0;

    template<typename T>
    bool isA() const;

    virtual bool isNumeric() const = 0;

    template<typename T>
    void set(CLS * obj, const T & value);

    template<typename T>
    const T & get(const CLS * obj);

  protected:
    virtual bool isAImpl(const std::type_info& ti) const = 0;
    virtual void * getImpl(CLS * obj, const std::type_info& ti) const = 0;
    virtual void setImpl(CLS * obj, const std::type_info& ti, const void * value) const = 0;

    template<typename T>
    static double asDouble(const T & value, std::true_type isNum)
    {
      return (double)value;
    };

    template<typename T>
    static double asDouble(const T & value, std::false_type isNotNum)
    {
      return 0.0;
    };

  private:
    std::string name;
  };

  template<typename CLS, typename T>
  class SimulMember : public SimulMemberBase<CLS>
  {
  public:
    typedef T member_type;
    typedef member_type CLS::*pointer_to_member_type;
    SimulMember(pointer_to_member_type member, const std::string & name);
    virtual bool isNumeric() const override;
    virtual void write(std::ostream & ost, const CLS * obj) const override;
    virtual void read(std::istream & ist, CLS * obj) const override;
    virtual void sort(std::vector<CLS> & data) const override;
    virtual double sum(const std::vector<CLS> & data) const override;
    virtual double sum2(const std::vector<CLS> & data, double offset=0) const override;
    virtual void copy(CLS * a, const CLS * b) const override;
  protected:
    virtual bool isAImpl(const std::type_info& ti) const override;
    virtual void * getImpl(CLS * obj, const std::type_info& ti) const override;
    virtual void setImpl(CLS * obj, const std::type_info& ti, const void * value) const override;
  private:
    pointer_to_member_type member;
  };
}

///////////////////////////////////////////////////////////////////////////////
template<typename CLS>
Lisp::SimulMemberBase<CLS>::SimulMemberBase(const std::string & _name)
  : name(_name)
{
}

template<typename CLS>
inline const std::string& Lisp::SimulMemberBase<CLS>::getName() const
{
  return name;
}

template<typename CLS>
template<typename T>
bool Lisp::SimulMemberBase<CLS>::isA() const
{
  return isAImpl(typeid(T));
}

template<typename CLS>
template<typename T>
void Lisp::SimulMemberBase<CLS>::set(CLS * obj, const T & value)
{
  setImpl(obj, typeid(T), (void*)&value);
}

template<typename CLS>
template<typename T>
const T & Lisp::SimulMemberBase<CLS>::get(const CLS * obj)
{
  return *static_cast<T*>(getImpl(obj, typeid(T)));
}

template<typename CLS, typename T>
Lisp::SimulMember<CLS, T>::SimulMember(pointer_to_member_type _member, const std::string & name)
  : SimulMemberBase<CLS>(name), member(_member)
{
}

template<typename CLS, typename T>
bool Lisp::SimulMember<CLS, T>::isNumeric() const
{
  return std::is_arithmetic<T>::value;
}

template<typename CLS, typename T>
void Lisp::SimulMember<CLS, T>::write(std::ostream & ost, const CLS * obj) const
{
  ost << obj->*member;
}

template<typename CLS, typename T>
void Lisp::SimulMember<CLS, T>::read(std::istream & ist, CLS * obj) const
{
  ist >> obj->*member;
}

template<typename CLS, typename T>
void Lisp::SimulMember<CLS, T>::sort(std::vector<CLS> & data) const
{
  std::sort(data.begin(),
            data.end(),
            [this](const CLS & a, const CLS & b) -> bool {
              return a.*member < b.*member;
            });
}

template<typename CLS, typename T>
void Lisp::SimulMember<CLS, T>::copy(CLS * a, const CLS * b) const
{
  a->*member = b->*member;
}

template<typename CLS, typename T>
bool Lisp::SimulMember<CLS, T>::isAImpl(const std::type_info& ti) const
{
  return std::type_index(ti) == std::type_index(typeid(T));
}

template<typename CLS, typename T>
void * Lisp::SimulMember<CLS, T>::getImpl(CLS * obj, const std::type_info& ti) const
{
  if(std::type_index(ti) != std::type_index(typeid(T)))
  {
    throw std::bad_cast();
  }
  return (void*)(&(obj->*member));
}

template<typename CLS, typename T>
void Lisp::SimulMember<CLS, T>::setImpl(CLS * obj, const std::type_info& ti, const void * value) const
{
  if(std::type_index(ti) != std::type_index(typeid(T)))
  {
    throw std::bad_cast();
  }
  obj->*member = *static_cast <const T*>(value);
}

template<typename CLS, typename T>
double Lisp::SimulMember<CLS, T>::sum(const std::vector<CLS> & data) const
{
  double ret = 0.0;
  for(const CLS & obj : data)
  {
    ret += SimulMemberBase<CLS>::asDouble(obj.*member, std::is_arithmetic<T>{});
  }
  return ret;
}

template<typename CLS, typename T>
double Lisp::SimulMember<CLS, T>::sum2(const std::vector<CLS> & data, double offset) const
{
  double ret = 0.0;
  for(const CLS & obj : data)
  {
    double x = SimulMemberBase<CLS>::asDouble(obj.*member, std::is_arithmetic<T>{});
    ret += (x - offset) * (x - offset);
  }
  return ret;

}
