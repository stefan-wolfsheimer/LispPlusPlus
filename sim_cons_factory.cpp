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
#include <simul/lisp_sim_cons_factory.h>

int main(int argc, const char ** argv)
{
  using SizeValue = Cli::Value<std::size_t>;
  using DoubleValue = Cli::Value<double>;
  using Flag = Cli::Flag;
  std::size_t targetNumRootConses = 10;
  std::size_t targetNumBulkConses = 100;
  double targetEdgeFraction = 0.5;
  std::size_t numSteps = 100;
  std::size_t numRuns = 1;
  std::size_t seed = 1;

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
  parser.add(SizeValue::make(numRuns,
                             "num-runs",
                             Cli::Doc("number of runs")));
  parser.add(SizeValue::make(seed,
                             "seed",
                             Cli::Doc("random number generator seed")));

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
  srand (seed);

  for(std::size_t run = 0; run < numRuns; run++)
  {
    Lisp::SimConsFactory simConsFactory;
    simConsFactory.setTargetNumRootConses(targetNumRootConses);
    simConsFactory.setTargetNumBulkConses(targetNumBulkConses);
    simConsFactory.setTargetEdgeFraction(targetEdgeFraction);
    simConsFactory.setNumSteps(numSteps);
    simConsFactory.run();
  }
  return 0;
}
