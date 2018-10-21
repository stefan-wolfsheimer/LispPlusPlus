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

namespace Lisp
{
  template<typename CLS>
  class SimulMemberBase
  {
  public:
    SimulMemberBase(const std::string & _name);
    virtual ~SimulMemberBase() {}
    inline const std::string& getName() const;
    virtual void stream(std::ostream & ost, const CLS * obj) const = 0;
    virtual void sort(std::vector<CLS> & data) const = 0;
    virtual void copy(CLS * a, const CLS * b) const = 0;
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
    virtual void stream(std::ostream & ost, const CLS * obj) const override;
    virtual void sort(std::vector<CLS> & data) const override;
    virtual void copy(CLS * a, const CLS * b) const override;
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

template<typename CLS, typename T>
Lisp::SimulMember<CLS, T>::SimulMember(pointer_to_member_type _member, const std::string & name)
  : SimulMemberBase<CLS>(name), member(_member)
{
}

template<typename CLS, typename T>
void Lisp::SimulMember<CLS, T>::stream(std::ostream & ost, const CLS * obj) const
{
  ost << obj->*member;
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
