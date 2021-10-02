/******************************************************************************
Copyright (c) 2021, Stefan Wolfsheimer

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
#include <lisp/util/argparse.h>
#include <lisp/util/xmalloc.h>
#include <lisp/simul/gc_sim_runner.h>
#include <lisp/core/gc_stat.h>

void run_all(lisp_gc_sim_runner_t * runner, size_t num_runs)
{
  lisp_gc_stat_t * stat;
  size_t i;
  stat = MALLOC(runner->num_steps * sizeof(lisp_gc_stat_t));
  for(i = 0; i < num_runs; i++)
  {
    lisp_gc_sim_runner_run(runner, stat);
  }
  FREE(stat);
}

int main(int argc, const char ** argv)
{
  arg_parser_t parser;
  lisp_gc_sim_runner_t runner;
  size_t seed;
  size_t num_runs;
  int ret;
  int help;
  ret = 0;
  seed = 1;
  num_runs = 1;
  help = 0;
  init_arg_parser(&parser);
  lisp_init_gc_sim_runner(&runner);
  arg_parser_add_flag(&parser,
                      &help,
                      'h',
                      "--help",
                      "show help");
  arg_parser_add_size(&parser,
                      &seed,
                      '\0',
                      "--seed",
                      0,
                      "random number seed");
  arg_parser_add_size(&parser,
                      &num_runs,
                      '\0',
                      "--num_runs",
                      0,
                      "number of runs");
  arg_parser_add_size(&parser,
                      &runner.num_edge_rewire_steps,
                      '\0',
                      "--num_edge_rewire_steps",
                      0,
                      "Number of attempts to set / unset edges after each step");
  arg_parser_add_size(&parser,
                      &runner.num_bulk_steps,
                      '\0',
                      "--num_bulk_steps",
                      0,
                      "Number of attempts to add / remove bulk");
  arg_parser_add_double(&parser,
                        &runner.mean_number_of_cells,
                        '\0',
                        "--mean_number_of_cells",
                        0,
                        "Mean number of target number of cell per node "
                        "(poisson distribution)");
  arg_parser_add_size(&parser,
                      &runner.num_steps,
                      '\0', "--num_steps",
                      0,
                      "number of steps per run");
  arg_parser_add_size(&parser,
                      &runner.recycle_steps,
                      '\0',
                      "--recycle_steps",
                      0,
                      "number of recycle steps");
  arg_parser_add_size(&parser,
                      &runner.garbage_steps,
                      '\0',
                      "--garbage_steps",
                      0,
                      "number of garbage steps");
  arg_parser_add_size(&parser,
                      &runner.num_bulk_steps,
                      '\0',
                      "--bulk_steps",
                      0,
                      "number of garbage steps");
  arg_parser_add_size(&parser,
                      &runner.num_edge_rewire_steps,
                      '\0',
                      "--edge_rewire_steps",
                      0,
                      "number of edge rewire steps");
  arg_parser_add_size(&parser,
                      &runner.target_num_root,
                      'r',
                      "--num_root",
                      0,
                      "target number of root objects");
  arg_parser_add_size(&parser,
                      &runner.target_num_bulk,
                      'b',
                      "--num_bulk",
                      0,
                      "target number of bulk objects");
  arg_parser_add_double(&parser,
                        &runner.target_child_fraction,
                        'c',
                        "--child_fraction",
                        0,
                        "target edge fraction\n"
                        "0.0: minimal number of children\n"
                        "1.0: maximal number of children");

  if(arg_parser_parse(&parser, argc - 1, argv + 1))
  {
    ret = 1;
  }
  if(help || ret)
  {
    arg_parser_print_help(stdout, argv[0], &parser);
  }
  else
  {
  }
  free_arg_parser(&parser);

  run_all(&runner, num_runs);

  lisp_free_gc_sim_runner(&runner);
  return ret;
}
