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
#include <cli.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <simul/lisp_sim_cons_factory.h>

using SizeValue = Cli::Value<std::size_t>;
using MultipleSizeValue = Cli::MultipleValue<std::size_t>;
using StringValue = Cli::Value<std::string>;
using DoubleValue = Cli::Value<double>;
using Flag = Cli::Flag;

using SimConsFactoryRecord = Lisp::SimConsFactoryRecord;
using SimConsFactorySeries = std::vector<SimConsFactoryRecord>;

static bool checkQuantiles(const std::vector<std::size_t> & quantiles,
                           std::size_t numRuns)
{
  bool ok = true;
  for(auto q : quantiles)
  {
    if(q > numRuns)
    {
      std::cerr << "Quantile " << q << " larger than num-runs ("
                << numRuns << ")";
      ok = false;
    }
  }
  return ok;
}

static std::string sizeToStringWithZeroFill(std::size_t n, std::size_t w)
{
  std::stringstream ss;
  ss << std::setw(w) << std::setfill('0') << n;
  return ss.str();
}

static void writeResults(std::string & outputFileName,
                         const std::vector<SimConsFactorySeries> & results)
{
  std::string runPattern("{RUN}");
  if(outputFileName.empty())
  {
    for(auto series : results)
    {
      std::cout << series;
    }
  }
  else
  {
    std::size_t pos = outputFileName.find(runPattern);
    if(pos == std::string::npos)
    {
      std::ofstream outfile(outputFileName.c_str());
      for(auto series : results)
      {
        outfile << series;
      }
      outfile.close();
    }
    else
    {
      for(std::size_t i = 0; i < results.size(); i++)
      {
        std::string tmp(outputFileName);
        tmp.replace(pos,
                    runPattern.size(),
                    sizeToStringWithZeroFill(i+1, 3).c_str());
        std::ofstream outfile(tmp.c_str());
        outfile << results[i];
        outfile.close();
      }
    }
  }
}

int main(int argc, const char ** argv)
{

  std::size_t targetNumRootConses = 10;
  std::size_t targetNumBulkConses = 100;
  double targetEdgeFraction = 0.5;
  std::size_t numSteps = 100;
  std::size_t numEdgeRewire = 3;
  std::size_t numBulkConsesSteps = 3;
  std::size_t numRuns = 1;
  std::size_t seed = 1;
  std::size_t garbageSteps = 1;
  std::size_t recycleSteps = 1;

  std::vector<std::size_t> quantiles;

  std::string quantileFile;
  std::string outputFiles;

  Cli::Parser parser("sim_cons_factory",
                     "simulate Cons garbage collector");
  parser.add(Flag::make('h', "help", Cli::Doc("show help")));
  parser.add(SizeValue::make(targetNumRootConses,
                             'r', "num-root-conses",
                             Cli::Doc("target number of root conses")));
  parser.add(SizeValue::make(targetNumBulkConses,
                             'b', "num-bulk-conses",
                             Cli::Doc("target number of bulk conses")));
  parser.add(DoubleValue::make(targetEdgeFraction,
                               'f', "edge-fraction",
                               Cli::Doc("target edge fraction\n"
                                        "0.0: minimal number of edges\n"
                                        "1.0: maximal number of edges")));
  parser.add(SizeValue::make(numSteps,
                             'n', "num-steps",
                             Cli::Doc("number of steps")));
  parser.add(SizeValue::make(numEdgeRewire,
                             "num-edge-rewire-steps",
                             Cli::Doc("Number of attempts to set / unset edges after each step")));
  parser.add(SizeValue::make(numBulkConsesSteps,
                             "num-bulk-conses-steps",
                             Cli::Doc("Number of attempts to set / unset edges after each step")));
  parser.add(SizeValue::make(garbageSteps,
                             "garbage-steps",
                             Cli::Doc("Number of times garbage is collected on each step.\n"
                                      "(parameter of Lisp::ConsFactory)")));
  parser.add(SizeValue::make(recycleSteps,
                             "recycle-steps",
                             Cli::Doc("Number of times conses are recycled on each step.\n"
                                      "(parameter of Lisp::ConsFactory)")));
  parser.add(SizeValue::make(numRuns,
                             "num-runs",
                             Cli::Doc("number of runs")));
  parser.add(MultipleSizeValue::make(quantiles,
                                     'q',
                                     "quantiles",
                                     Cli::Doc("Accumulate set of quantiles.\n"
                                              "Values must be in [0...num-runs)\n")));
  parser.add(SizeValue::make(seed,
                             "seed",
                             Cli::Doc("random number generator seed")));
  parser.add(StringValue::make(outputFiles,
                               'o',
                               "output",
                               Cli::Doc("Filename of the time series\n"
                                        "It can contain {RUN} for a "
                                        "placeholder for the run number.\n"
                                        "If filename does not contain {RUN} "
                                        "all runs are written to one file")));
  parser.add(StringValue::make(quantileFile,
                               "quantile-output",
                               Cli::Doc("Filename for generated quantiles")));

  std::vector<std::string> err;
  if(!parser.parse(argc, argv, err))
  {
    for(auto line : err)
    {
      std::cerr << line << std::endl;
    }
    parser.printHelp(std::cerr);
    return 8;
  }
  if(parser.isSet("help"))
  {
    parser.printHelp(std::cout);
    return 0;
  }
  if(!checkQuantiles(quantiles, numRuns))
  {
    return 8;
  }

  srand (seed);
  std::vector<SimConsFactorySeries> results;
  for(std::size_t run = 0; run < numRuns; run++)
  {
    Lisp::SimConsFactory simConsFactory;
    simConsFactory.setTargetNumRootConses(targetNumRootConses);
    simConsFactory.setTargetNumBulkConses(targetNumBulkConses);
    simConsFactory.setTargetEdgeFraction(targetEdgeFraction);
    simConsFactory.setNumSteps(numSteps);
    simConsFactory.setNumEdgeRewireSteps(numEdgeRewire);
    simConsFactory.setNumBulkConsesSteps(numBulkConsesSteps);
    simConsFactory.setGarbageSteps(garbageSteps);
    simConsFactory.setRecycleSteps(recycleSteps);
    results.push_back(simConsFactory.run());
  }
  writeResults(outputFiles, results);
  if(!quantiles.empty())
  {
    auto quantileResults = SimConsFactoryRecord::computeQuantiles(results,
                                                                  quantiles);
    if(quantileFile.empty())
    {
      std::cout << quantileResults;
    }
    else
    {
      std::ofstream outfile(quantileFile.c_str());
      outfile << quantileResults;
      outfile.close();
    }
  }
  return 0;
}
