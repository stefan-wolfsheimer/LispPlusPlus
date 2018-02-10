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
using SeriesType = SimConsFactoryRecord::SeriesType;
using QuantilesType = SimConsFactoryRecord::QuantilesType;
using QuantilesSeries = SimConsFactoryRecord::QuantilesSeries;
using Builder = SimConsFactoryRecord::Builder;

Builder SimConsFactoryRecord::getBuilder()
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
    .member<double>(&SimConsFactoryRecord::edgeFraction,"edgeFraction", 0);
  return builder;
}

SimConsFactoryRecord::ConsFractions::Builder SimConsFactoryRecord::ConsFractions::getBuilder()
{
  SimConsFactoryRecord::ConsFractions::Builder builder;
  builder
    .member<std::size_t>(&SimConsFactoryRecord::ConsFractions::step, "step", 0)
    .member<double>(&SimConsFactoryRecord::ConsFractions::rootConsFraction, "rootConsFraction", 0.0)
    .member<double>(&SimConsFactoryRecord::ConsFractions::bulkConsFraction, "bulkConsFraction", 0.0)
    .member<double>(&SimConsFactoryRecord::ConsFractions::voidConsFraction, "voidConsFraction", 0.0)
    .member<double>(&SimConsFactoryRecord::ConsFractions::freeConsFraction, "freeConsFraction", 0.0)
    .member<double>(&SimConsFactoryRecord::ConsFractions::numberOfConses,   "numberOfConses", 0.0);
  return builder;
}

SimConsFactoryRecord::SimConsFactoryRecord()
{
  for(auto memb : SimConsFactoryRecord::getBuilder())
  {
    memb->initialize(*this);
  }
}

SimConsFactoryRecord::ConsFractions::ConsFractions()
{
  for(auto memb : SimConsFactoryRecord::ConsFractions::getBuilder())
  {
    memb->initialize(*this);
  }
}

template<typename T>
static std::ostream& streamOut(std::ostream & ost, const T & rec)
{
  bool first = true;
  for(auto member : T::getBuilder())
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

template<typename T>
static std::ostream& streamOutVector(std::ostream & ost, const std::vector<T> & data)
{
  bool first = true;
  for(auto member : T::getBuilder())
  {
    if(!first)
    {
      ost.put(',');
    }
    ost << member->getName();
    first = false;
  }
  ost << std::endl;
  for(const T & rec : data)
  {
    streamOut<T>(ost, rec);
  }
  return ost;
}


std::ostream& operator<<(std::ostream & ost, const SimConsFactoryRecord & rec)
{
  return streamOut<SimConsFactoryRecord>(ost, rec);
}

std::ostream& operator<<(std::ostream & ost, const Lisp::SimConsFactoryRecord::ConsFractions & rec)
{
  return streamOut<Lisp::SimConsFactoryRecord::ConsFractions>(ost, rec);
}

std::ostream& operator<<(std::ostream & ost, const std::vector<SimConsFactoryRecord> & data)
{
  return streamOutVector<SimConsFactoryRecord>(ost, data);
}

std::ostream& operator<<(std::ostream & ost, const std::vector<Lisp::SimConsFactoryRecord::ConsFractions> & data)
{
  return streamOutVector<SimConsFactoryRecord::ConsFractions>(ost, data);
}

std::ostream& operator<<(std::ostream & ost, const QuantilesSeries & q)
{
  bool first = true;
  for(const QuantilesType & quantiles : q)
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
        first = false;
      }
      ost << std::endl;
    }
    if(!quantiles.empty())
    {
      ost << quantiles.begin()->second.step;
      for(auto member : SimConsFactoryRecord::getBuilder())
      {
        if(member->getName() != "step")
        {
          for(auto pair : quantiles)
          {
            ost << ",";
            member->streamOut(ost, pair.second);
          }
        }
      }
      ost << std::endl;
    }
  }
  return ost;
}

template<typename T>
void SimConsFactoryRecord::computeQuantiles(std::size_t i, const std::vector<SeriesType> & runs, QuantilesType & ret)
{
  for(auto member : SimConsFactoryRecord::getBuilder())
  {
    if(member->getTypeInfo() == typeid(T))
    {
      std::vector<T> values;
      for(auto run : runs)
      {
        values.push_back(member->bind<T>(run[i]));
      }
      std::sort(values.begin(), values.end());
      for(auto & pair : ret)
      {
        member->bind<T>(pair.second) = values[pair.first];
      }
    }
  }
}

QuantilesType SimConsFactoryRecord::computeQuantiles(std::size_t i,
                                                     const std::vector<SeriesType> & runs,
                                                     const std::vector<std::size_t> & qs)
{
  QuantilesType ret;
  for(std::size_t q : qs)
  {
    ret[q] = SimConsFactoryRecord();
  }
  computeQuantiles<std::size_t>(i, runs, ret);
  computeQuantiles<double>(i, runs, ret);
  return ret;
}

std::size_t SimConsFactoryRecord::getLengthOfSeries(const std::vector<SeriesType> & runs)
{
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
  return s;
}

std::vector<QuantilesType> SimConsFactoryRecord::computeQuantiles(const std::vector<SeriesType> & runs,
                                                                  const std::vector<std::size_t> & qs)
{
  std::vector<QuantilesType> ret;
  std::size_t len = getLengthOfSeries(runs);
  for(std::size_t i = 0; i < len; i++)
  {
    ret.push_back(computeQuantiles(i, runs, qs));
  }
  return ret;
}

std::vector<SimConsFactoryRecord::ConsFractions> SimConsFactoryRecord::computeAverageFractions(const std::vector<SeriesType> & runs)
{
  std::size_t len = getLengthOfSeries(runs);
  std::vector<SimConsFactoryRecord::ConsFractions> ret(len);
  for(auto & run : runs)
  {
    for(std::size_t i = 0; i < len; i++)
    {
      ret[i].step = run[i].step;
      ret[i].rootConsFraction += run[i].numRootConses;
      ret[i].bulkConsFraction += run[i].numBulkConses;
      ret[i].voidConsFraction += run[i].numVoidConses;
      ret[i].freeConsFraction += run[i].numFreeConses;
      ret[i].numberOfConses   += run[i].numRootConses + run[i].numBulkConses + run[i].numVoidConses + run[i].numFreeConses;
    }
  }
  for(std::size_t i = 0; i < len; i++)
  {
    double z = ret[i].rootConsFraction + ret[i].bulkConsFraction + ret[i].voidConsFraction + ret[i].freeConsFraction;
    if(z > 1e-8)
    {
      ret[i].rootConsFraction/= z;
      ret[i].bulkConsFraction/= z;
      ret[i].voidConsFraction/= z;
      ret[i].freeConsFraction/= z;
      if(runs.size())
      {
        ret[i].numberOfConses/= runs.size();
      }
    }
  }
  return ret;
}
