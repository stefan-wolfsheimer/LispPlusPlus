#include "gc_sim.h"
#include "gc_sim_runner.h"
#include <lisp/core/gc_stat.h>

int lisp_init_gc_sim_runner(lisp_gc_sim_runner_t * runner)
{
  runner->mean_number_of_cells = 4;
  runner->target_num_root = 10;
  runner->target_num_bulk = 10;
  runner->target_child_fraction = 0.5;
  runner->num_edge_rewire_steps = 5;
  runner->num_bulk_steps = 3;
  runner->garbage_steps = 1;
  runner->recycle_steps = 1;
  runner->num_steps = 100;
  return 0;
}

int lisp_free_gc_sim_runner(lisp_gc_sim_runner_t * runner)
{
  return 0;
}

void lisp_gc_sim_runner_run(lisp_gc_sim_runner_t * runner,
                            lisp_gc_stat_t * stat)
{
  size_t i, j;
  size_t num_steps;
  size_t num_bulk_steps;
  size_t num_edge_rewire_steps;
  lisp_gc_sim_t sim;
  num_steps = runner->num_steps;
  num_bulk_steps = runner->num_bulk_steps;
  num_edge_rewire_steps = runner->num_edge_rewire_steps;

  lisp_init_gc_sim(&sim);
  for(i = 0; i < num_steps; i++)
  {
    /* @todo step root */
    for(j = 0; j < num_bulk_steps; j++)
    {
      /* @todo step bulk */
    }
    for(j = 0; j < num_edge_rewire_steps; j++)
    {
      /* @todo step rewire */
    }
    lisp_vm_gc_get_stats(&sim.vm, &stat[i]);
    /* lisp_gc_stat_print(stdout, &stat[i]); */
  }
  lisp_free_gc_sim(&sim);
}
