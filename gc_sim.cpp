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
#include <lpp/simul/gc_sim_runner.h>
#include <lpp/simul/gc_sim_record.h>
#include <lpp/simul/sim_quantile.h>


int main(int argc, const char ** argv)
{
  std::size_t numRuns = 10;
  unsigned seed = 1.0;
  std::default_random_engine generator (seed);
  std::vector<std::vector<Lisp::GcSimRecord>> runs;
  for(std::size_t i = 0; i < numRuns; i++)
  {
    Lisp::GcSimRunner<std::default_random_engine> runner(generator);
    std::vector<Lisp::GcSimRecord> run = runner.run();
    std::cout << run << std::endl;
    runs.push_back(std::move(run));
  }
  Lisp::Quantile<Lisp::GcSimRecord> quantile({2,4,6,8,9},
                                             Lisp::GcSimRecordMembers().members,
                                             runs);
  std::cout << "step,";
  quantile.streamHeader(std::cout) << std::endl;
  for(std::size_t i = 0; i < quantile.numSteps(); i++)
  {
    std::cout << i << ",";
    quantile.streamRow(std::cout, i) << std::endl;
  }
  return 0;
}
