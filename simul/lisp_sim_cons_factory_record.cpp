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
#include "lisp_sim_cons_factory_record.h"
#include <algorithm>

using SimConsFactoryRecord = Lisp::SimConsFactoryRecord;

SimConsFactoryRecord::Builder SimConsFactoryRecord::getBuilder()
{
  Builder builder;
  builder
    .member<std::size_t>(&SimConsFactoryRecord::step, "step", 0)
    .member<std::size_t>(&SimConsFactoryRecord::numRootConses, "numRootConses", 0)
    .member<std::size_t>(&SimConsFactoryRecord::numBulkConses, "numBulkConses",0)
    .member<std::size_t>(&SimConsFactoryRecord::numReachableConses, "numReachableConses", 0)
    .member<std::size_t>(&SimConsFactoryRecord::numVoidConses, "numVoidConses",0)
    .member<std::size_t>(&SimConsFactoryRecord::numFreeConses, "numFreeConses",0)
    .member<std::size_t>(&SimConsFactoryRecord::numEdges, "numEdges",0)
    .member<double>(&SimConsFactoryRecord::expectedNumEdges,"expectedNumEdges",0) 
    .member<double>(&SimConsFactoryRecord::edgeFraction,"edgeFraction", 0)
    .member<double>(&SimConsFactoryRecord::rootConsFraction, "rootConsFraction", 0)
    .member<double>(&SimConsFactoryRecord::bulkConsFraction, "bulkConsFraction", 0)
    .member<double>(&SimConsFactoryRecord::reachableConsFraction,"reachableConsFraction", 0) 
    .member<double>(&SimConsFactoryRecord::voidConsFraction, "voidConsFraction", 0)
    .member<double>(&SimConsFactoryRecord::freeConsFraction, "freeConsFraction", 0);
  return builder;
}

std::vector<SimConsFactoryRecord::size_t_ptr>
SimConsFactoryRecord::getSizeTypeValues()
{
  return std::vector<size_t_ptr>({
      &SimConsFactoryRecord::step,
      &SimConsFactoryRecord::numRootConses,
      &SimConsFactoryRecord::numBulkConses,
      &SimConsFactoryRecord::numReachableConses,
      &SimConsFactoryRecord::numVoidConses,
      &SimConsFactoryRecord::numFreeConses,
      &SimConsFactoryRecord::numEdges });
}

std::vector<SimConsFactoryRecord::double_ptr>
SimConsFactoryRecord::getDoubleValues()
{
  return std::vector<double_ptr>({
      &SimConsFactoryRecord::expectedNumEdges,
      &SimConsFactoryRecord::edgeFraction,
      &SimConsFactoryRecord::rootConsFraction,
      &SimConsFactoryRecord::bulkConsFraction,
      &SimConsFactoryRecord::reachableConsFraction,
      &SimConsFactoryRecord::voidConsFraction,
      &SimConsFactoryRecord::freeConsFraction});
}

SimConsFactoryRecord::SimConsFactoryRecord()
{
  for(auto ptr : getSizeTypeValues())
  {
    this->*ptr = 0;
  }
  for(auto ptr : getDoubleValues())
  {
    this->*ptr = 0.0;
  }
}

std::ostream& operator<<(std::ostream & ost,
                         const SimConsFactoryRecord & rec)
{
  bool first = true;
  for(auto member : SimConsFactoryRecord::getBuilder())
  {
    if(!first)
    {
      ost.put(',');
    }
    member->streamOut(ost, rec);
    first = false;
  }
  ost << std::endl;
  return ost;
}

std::ostream& operator<<(std::ostream & ost,
                         const std::vector<SimConsFactoryRecord> & data)
{
  bool first = true;
  for(auto member : SimConsFactoryRecord::getBuilder())
  {
    if(!first)
    {
      ost.put(',');
    }
    ost << member->getName();
    first = false;
  }
  ost << std::endl;
  for(const SimConsFactoryRecord & rec : data)
  {
    ost << rec;
  }
  return ost;
}

std::ostream& operator<<(std::ostream & ost,
                         const Lisp::SimConsFactoryRecord::QuantilesSeries & q)
{
  bool first = true;
  for(const SimConsFactoryRecord::QuantilesType & quantiles : q)
  {
    if(first)
    {
      ost <<  "step";
      for(auto member : SimConsFactoryRecord::getBuilder())
      {
        if(member->getName() != "step")
        {
          for(auto pair : quantiles)
          {
            ost << "," << member->getName() << "_Q" << pair.first;
          }
        }
        ost << std::endl;
        first = false;
      }
    }
    if(!quantiles.empty())
    {
      ost << quantiles.begin()->second.step;
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.numRootConses;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.numBulkConses;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.numReachableConses;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.numVoidConses;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.numFreeConses;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.numEdges;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.expectedNumEdges;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.edgeFraction;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.rootConsFraction;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.bulkConsFraction;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.reachableConsFraction;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.voidConsFraction;
      }
      for(auto pair : quantiles)
      {
        ost << "," << pair.second.freeConsFraction;
      }
      ost << std::endl;
    }
  }
  return ost;
}


SimConsFactoryRecord::QuantilesType
SimConsFactoryRecord::computeQuantiles(std::size_t i,
                                       const std::vector<SimConsFactoryRecord::SeriesType> & runs,
                                       const std::vector<std::size_t> & qs)
{
  QuantilesType ret;
  for(std::size_t q : qs)
  {
    ret[q] = SimConsFactoryRecord();
  }
  for(auto ptr : getSizeTypeValues())
  {
    std::vector<std::size_t> values;
    for(auto run : runs)
    {
      values.push_back(run[i].*ptr);
    }
    std::sort(values.begin(), values.end());
    for(std::size_t q : qs)
    {
      ret[q].*ptr = values[q];
    }
  }
  for(auto ptr : getDoubleValues())
  {
    std::vector<double> values;
    for(auto run : runs)
    {
      values.push_back(run[i].*ptr);
    }
    std::sort(values.begin(), values.end());
    for(std::size_t q : qs)
    {
      ret[q].*ptr = values[q];
    }
  }
  return ret;
}

std::vector<SimConsFactoryRecord::QuantilesType>
SimConsFactoryRecord::computeQuantiles(const std::vector<SeriesType> & runs,
                                       const std::vector<std::size_t> & qs)
{
  std::vector<QuantilesType> ret;
  std::size_t s = 0;
  bool first = true;
  for(auto & run : runs)
  {
    if(first)
    {
      first = false;
      s = run.size();
    }
    else if(s != run.size())
    {
      throw std::logic_error("runs with different lengths in set of runs");
    }
  }
  for(std::size_t i = 0; i < s; i++)
  {
    ret.push_back(computeQuantiles(i, runs, qs));
  }
  return ret;
}
