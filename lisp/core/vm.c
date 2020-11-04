#include "vm.h"
#include "error.h"
#include "cell.h"
#include "cons.h"
#include "tid.h"

int lisp_init_vm(lisp_vm_t * vm)
{
  return lisp_init_gc(&vm->gc);
}

int lisp_free_vm(lisp_vm_t * vm)
{
  return lisp_free_gc(&vm->gc);
}

int lisp_make_cons(lisp_vm_t * vm,
                   lisp_cell_t * cell,
                   const lisp_cell_t * car,
                   const lisp_cell_t * cdr)
{
  cell->type_id = LISP_TID_CONS;
  cell->data.cons = lisp_gc_alloc_root_cons(&vm->gc);
  //lisp_cell_t car;
  //lisp_cell_t cdr;
  return LISP_OK;
}
