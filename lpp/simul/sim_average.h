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
#include <lpp/simul/sim_run_statistics.h>
#include <vector>
#include <memory>

namespace Lisp
{
  template<typename CLS>
  class Average : public RunStatistics
  {
  public:
    Average(const std::vector<std::shared_ptr<SimulMemberBase<CLS>>> & _fields,
            const std::vector<std::vector<CLS>> & runs);
    std::size_t numSteps() const override;
    std::ostream & streamHeader(std::ostream & ost) const override;
    std::ostream & streamRow(std::ostream & ost, std::size_t i) const override;
  private:
    struct Record
    {
      std::size_t step;
      std::size_t n;
      double avg;
      double avg2;
      Record(std::size_t i, std::shared_ptr<SimulMemberBase<CLS>> field, const std::vector<CLS> & data);
    };
    std::vector<std::shared_ptr<SimulMemberBase<CLS>>> fields;
    std::vector<std::vector<Record>> averages;
  };
}

///////////////////////////////////////////////////////////////////////////////
//
// implementation
//
///////////////////////////////////////////////////////////////////////////////

template<typename CLS>
Lisp::Average<CLS>::Record::Record(std::size_t i, std::shared_ptr<SimulMemberBase<CLS>> field, const std::vector<CLS> & data)
{
  step = i;
  n = data.size();
  avg = field->sum(data);
  if(n)
  {
    avg/= n;
  }
  avg2 = field->sum2(data, avg);
  if(n)
  {
    avg2/= n;
  }
}

template<typename CLS>
Lisp::Average<CLS>::Average(const std::vector<std::shared_ptr<SimulMemberBase<CLS>>> & _fields,
                            const std::vector<std::vector<CLS>> & runs)
  : fields(_fields)
{
  std::size_t numSteps = 0;
  // @todo better solution here
  for(const auto & run : runs)
  {
    if(numSteps == 0)
    {
      numSteps = run.size();
    }
    else
    {
      if(numSteps != run.size())
      {
        throw std::logic_error("runs not transposible");
      }
    }
  }
  averages.reserve(numSteps);
  for(std::size_t i = 0; i < numSteps; i++)
  {
    std::vector<CLS> data;
    data.reserve(runs.size());
    for(std::size_t r = 0; r < runs.size(); r++)
    {
      data.push_back(runs[r][i]);
    }
    std::vector<Record> records;
    for(auto field : fields)
    {
      if(field->isNumeric())
      {
        records.push_back(Record(i, field, data));
      }
    }
    averages.push_back(records);
  }
}

template<typename CLS>
std::size_t Lisp::Average<CLS>::numSteps() const
{
  return averages.size();
}

template<typename CLS>
std::ostream & Lisp::Average<CLS>::streamHeader(std::ostream & ost) const
{
  bool first = true;
  for(auto field : fields)
  {
    if(field->isNumeric())
    {
      if(first)
      {
        first = false;
      }
      else
      {
        ost << ",";
      }
      ost << field->getName() << "[avg]," << field->getName() << "[std]";
    }
  }
  return ost;
}

template<typename CLS>
std::ostream & Lisp::Average<CLS>::streamRow(std::ostream & ost, std::size_t i) const
{
  bool first = true;
  for(const auto & rec : averages[i])
  {
    if(first)
    {
      first = false;
    }
    else
    {
      ost << ",";
    }
    ost << rec.avg << "," << rec.avg2;
  }
  return ost;
}
