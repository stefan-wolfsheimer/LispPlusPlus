#include "gc_sim.h"
#include <lisp/core/error.h>

int lisp_init_gc_sim(lisp_gc_sim_t * sim)
{
  sim->root = NULL;
  sim->n_root = 0;
  return lisp_init_vm(&sim->vm);
}

int lisp_free_gc_sim(lisp_gc_sim_t * sim)
{
  return lisp_free_vm(&sim->vm);
}
