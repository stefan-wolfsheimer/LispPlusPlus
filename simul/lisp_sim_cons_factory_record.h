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
#include <csv/builder.h>
#include <cstddef>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <ostream>
#include <string>

namespace Lisp
{
  class ConsFactory;
  class ConsGraph;
  class Object;
  class Cons;

  struct SimConsFactoryRecord
  {
    typedef std::vector<SimConsFactoryRecord> SeriesType;
    typedef std::map<std::size_t, SimConsFactoryRecord> QuantilesType;
    typedef std::vector<QuantilesType> QuantilesSeries;
    typedef csv::Builder<SimConsFactoryRecord> Builder;
    std::size_t step;
    std::size_t numRootConses;
    std::size_t numBulkConses;
    std::size_t numReachableConses;
    std::size_t numVoidConses;
    std::size_t numFreeConses;
    std::size_t numEdges;
    double expectedNumEdges;
    double edgeFraction;
    double rootConsFraction;
    double bulkConsFraction;
    double reachableConsFraction;
    double voidConsFraction;
    double freeConsFraction;
    SimConsFactoryRecord();

    static QuantilesSeries computeQuantiles(const std::vector<SeriesType> & r,
                                            const std::vector<std::size_t> & q);
    static Builder getBuilder();
  private:
    typedef std::size_t SimConsFactoryRecord::*size_t_ptr;
    typedef double SimConsFactoryRecord::*double_ptr;
    static std::vector<size_t_ptr> getSizeTypeValues();
    static std::vector<double_ptr> getDoubleValues();

    static QuantilesType
    computeQuantiles(std::size_t i,
                     const std::vector<SeriesType> & runs,
                     const std::vector<std::size_t> & qs);
  };
}

std::ostream& operator<<(std::ostream &,
                         const Lisp::SimConsFactoryRecord & rec);

std::ostream& operator<<(std::ostream &,
                         const std::vector<Lisp::SimConsFactoryRecord> & data);

std::ostream& operator<<(std::ostream &,
                         const Lisp::SimConsFactoryRecord::QuantilesSeries &);
