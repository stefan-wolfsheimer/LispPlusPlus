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
#include <random>
#include <utility>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <typeindex>
#include <typeinfo>
#include <lpp/simul/gc_sim_runner.h>
#include <lpp/simul/gc_sim_record.h>
#include <lpp/simul/sim_quantile.h>
#include <cli.h>

using GcSimRunner = Lisp::GcSimRunner<std::default_random_engine>;

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

static void writeRun(std::shared_ptr<Cli::Value<std::string>> outputFileName,
                     std::size_t index,
                     const std::vector<Lisp::GcSimRecord> & run)
{
  if(outputFileName->isSet())
  {
    std::string runPattern("{RUN}");
    std::string fname = outputFileName->getValue();
    std::size_t pos = fname.find(runPattern);
    if(pos != std::string::npos)
    {
      std::stringstream ss;
      ss << std::setw(3) << std::setfill('0') << (index + 1);
      fname.replace(pos,
                    runPattern.size(),
                    ss.str().c_str());
    }
    std::ofstream outfile(fname.c_str());
    outfile << run << std::endl;
    outfile.close();
  }
  else
  {
    std::cout << run << std::endl;
  }
}

static void writeQuantiles(std::ostream & ost,
                           const Lisp::Quantile<Lisp::GcSimRecord> & quantiles_result)
{
  ost << "step,";
  quantiles_result.streamHeader(ost) << std::endl;
  for(std::size_t i = 0; i < quantiles_result.numSteps(); i++)
  {
    ost << i << ",";
    quantiles_result.streamRow(ost, i) << std::endl;
  }
}

static void writeQuantiles(std::shared_ptr<Cli::Value<std::string>> outputFileName,
                           const Lisp::Quantile<Lisp::GcSimRecord> & quantiles_result)
{
  if(outputFileName->isSet())
  {
    std::ofstream outfile(outputFileName->getValue().c_str());
    writeQuantiles(outfile, quantiles_result);
    outfile.close();
  }
  else
  {
    writeQuantiles(std::cout, quantiles_result);
  }
}

template<typename T>
bool configureRunnerArgument(GcSimRunner & runner, GcSimRunner::MemberType & member, const std::shared_ptr<Cli::Argument> & arg) 
{
  if(std::type_index(arg->getTypeInfo()) == std::type_index(typeid(T)))
  {
    member->set(&runner, arg->valueCast<T>());
    return true;
  }
  else
  {
    return false;
  }
}

void configureRunner(GcSimRunner & runner, const Cli::Parser & parser)
{
  for(auto param : runner.getParameters())
  {
    auto arg = parser.getArgument(param->getName());
    if(arg && arg->isSet())
    {
      configureRunnerArgument<std::size_t>(runner, param, arg) ||
        configureRunnerArgument<double>(runner, param, arg);
    }
  }
}

int main(int argc, const char ** argv)
{
  std::size_t seed = 1;
  std::size_t numRuns = 1;

  Cli::Parser parser("gc_sim",
                     "Simulate memory management");
  auto help = parser.addFlag('h', "help", Cli::Doc("show help"));

  // statistics
  auto argNumSteps = parser.addValue<std::size_t>('n', "num_steps",
                                               Cli::Doc("number of steps per run"));
  parser.addValue<std::size_t>(numRuns, "num_runs", Cli::Doc("number of runs"));

  auto quantiles = parser.addMultipleValue<std::size_t>('q',
                                                        "quantiles",
                                                        Cli::Doc("Accumulate set of quantiles.\n"
                                                                 "Values must be in [0...num-runs)\n"));
  parser.addValue<std::size_t>(seed, "seed",
                               Cli::Doc("random number generator seed"));
  auto outputFiles = parser.addValue<std::string>('o',
                                                  "output",
                                                  Cli::Doc("Filename of the time series\n"
                                                           "It can contain {RUN} for a "
                                                           "placeholder for the run number.\n"
                                                           "If filename does not contain {RUN} "
                                                           "all runs are written to one file"));
  auto quantileFile = parser.addValue<std::string>("quantile_output",
                                                   Cli::Doc("Filename for generated quantiles"));

  // simulation parameters
  parser.addValue<std::size_t>('r', "num_root",
                               Cli::Doc("target number of root objects"));
  parser.addValue<std::size_t>('b', "num_bulk",
                               Cli::Doc("target number of bulk objects"));

  parser.addValue<double>('c', "child_fraction",
                          Cli::Doc("target edge fraction\n"
                                   "0.0: minimal number of children\n"
                                   "1.0: maximal number of children"));
  parser.addValue<double>('n', "num_cells",
                          Cli::Doc("Mean number of target number of cell per node (posson distribution)"));

  parser.addValue<std::size_t>("num_edge_rewire_steps",
                               Cli::Doc("Number of attempts to set / unset edges after each step"));
  parser.addValue<std::size_t>("num_bulk_conses_steps",
                               Cli::Doc("Number of attempts to add / remove bulk"));
  // garbage collector parameters
  parser.addValue<std::size_t>("garbage_steps",
                               Cli::Doc("Number of times garbage is collected on each step.\n"
                                        "(parameter of Lisp::GarbageCollector)"));
  parser.addValue<std::size_t>("recycle_steps",
                               Cli::Doc("Number of times conses are recycled on each step.\n"
                                        "(parameter of Lisp::GarbageCollector)"));
#if 0
  // @todo
  parser.add(StringValue::make(consFractionFile,
                               "cons-fraction-output",
                               Cli::Doc("Filename for cons fraction statistics")));
#endif

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
  if(quantiles->isSet())
  {
    if(!checkQuantiles(quantiles->getValue(), numRuns))
    {
      return 8;
    }
  }
  GcSimRunner::RandomEngine generator (seed);
  std::vector<std::vector<Lisp::GcSimRecord>> runs;
  for(std::size_t i = 0; i < numRuns; i++)
  {
    GcSimRunner runner(generator);
    configureRunner(runner, parser);
    std::vector<Lisp::GcSimRecord> run = runner.run();
    writeRun(outputFiles, i, run);
    if(quantiles->isSet())
    {
      runs.push_back(std::move(run));
    }
    if(quantiles->isSet())
    {
      Lisp::Quantile<Lisp::GcSimRecord> quantiles_result(quantiles->getValue(),
                                                         Lisp::GcSimRecordMembers().members,
                                                         runs);
      writeQuantiles(quantileFile, quantiles_result);
    }
  }
  return 0;
}
