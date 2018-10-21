/******************************************************************************
Copyright (c) 2017-2018, Stefan Wolfsheimer

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
#include <lpp/simul/sim_member.h>
#include <vector>
#include <memory>

namespace Lisp
{
  template<typename CLS>
  class Quantile
  {
  public:
    Quantile(const std::vector<std::size_t> & _quantile,
             const std::vector<std::shared_ptr<SimulMemberBase<CLS>>> & _fields,
             const std::vector<std::vector<CLS>> & runs);
    std::size_t numSteps() const;
    std::ostream & streamHeader(std::ostream & ost) const;
    std::ostream & streamRow(std::ostream & ost, std::size_t i) const;
  private:
    std::vector<std::size_t> quantiles;
    std::vector<std::vector<CLS>> data;
    std::vector<std::shared_ptr<SimulMemberBase<CLS>>> fields;
    std::vector<CLS> computeQuantiles(const std::vector<std::vector<CLS>> & runs,
                                      std::size_t step);
  };
}

///////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////

template<typename CLS>
Lisp::Quantile<CLS>::Quantile(const std::vector<std::size_t> & _quantiles,
                              const std::vector<std::shared_ptr<SimulMemberBase<CLS>>> & _fields,
                              const std::vector<std::vector<CLS>> & runs)
             : fields(_fields), quantiles(_quantiles)
{
  if(!runs.empty())
  {
    std::size_t nsteps = runs.front().size();
    data.reserve(nsteps);
    for(std::size_t i = 0; i < nsteps; i++)
    {
      data.push_back(computeQuantiles(runs, i));
    }
  }
}

template<typename CLS>
std::vector<CLS> Lisp::Quantile<CLS>::computeQuantiles(const std::vector<std::vector<CLS>> & runs,
                                                       std::size_t step)
{
  std::vector<CLS> records;
  std::vector<CLS> ret;
  ret.reserve(quantiles.size());
  for(auto & run : runs)
  {
    records.push_back(run[step]);
  }
  for(const CLS & obj : runs.front())
  {
    ret.push_back(obj);
  }
  for(auto field : fields)
  {
    field->sort(records);
    for(std::size_t i = 0; i < quantiles.size(); i++)
    {
      field->copy(&ret[i], &records.at(quantiles[i]));
    }
  }
  return ret;
}

template<typename CLS>
std::size_t Lisp::Quantile<CLS>::numSteps() const
{
  return data.size();
}

template<typename CLS>
std::ostream & Lisp::Quantile<CLS>::streamHeader(std::ostream & ost) const
{
  bool first = true;
  for(std::size_t i = 0; i < quantiles.size(); i++)
  {
    for(auto field : fields)
    {
      if(first)
      {
        first = false;
      }
      else
      {
        ost << ",";
      }
      ost << field->getName() << "[" << quantiles[i] << "]";
    }
  }
  return ost;
}

template<typename CLS>
std::ostream & Lisp::Quantile<CLS>::streamRow(std::ostream & ost, std::size_t step) const
{
  bool first = true;
  for(std::size_t i = 0; i < quantiles.size(); i++)
  {
    for(auto field : fields)
    {
      if(first)
      {
        first = false;
      }
      else
      {
        ost << ",";
      }
      field->stream(ost, &data[step][i]);
    }
  }
  return ost;
}
