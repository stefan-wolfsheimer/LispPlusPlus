/******************************************************************************
Copyright (c) 2017-2021, Stefan Wolfsheimer

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
#ifndef __GC_SIM_RUNNER_H__
#define __GC_SIM_RUNNER_H__
#include <stddef.h>
#include <lisp/core/gc_stat.h>

typedef struct lisp_gc_sim_runner_t
{
  double mean_number_of_cells;
  size_t target_num_root;
  size_t target_num_bulk;
  double target_child_fraction;
  size_t num_edge_rewire_steps;
  size_t num_bulk_steps;
  size_t garbage_steps;
  size_t recycle_steps;
  size_t num_steps;
} lisp_gc_sim_runner_t;

int lisp_init_gc_sim_runner(lisp_gc_sim_runner_t * runner);
int lisp_free_gc_sim_runner(lisp_gc_sim_runner_t * runner);

void lisp_gc_sim_runner_run(lisp_gc_sim_runner_t * runner,
                            lisp_gc_stat_t * stat);

#endif
