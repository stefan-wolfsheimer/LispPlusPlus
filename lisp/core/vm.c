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

static int _lisp_make_cons_cell(lisp_vm_t * vm,
                                lisp_cell_t * target,
                                const lisp_cell_t * source)
{
  if(source)
  {
    if(LISP_IS_STORAGE_ATOM_TID(source->type_id))
    {
      target->type_id = source->type_id;
      target->data = source->data;
    }
    else
    {
      return LISP_NOT_IMPLEMENTED;
    }
  }
  else
  {
    target->type_id = LISP_TID_NIL;
  }
  return LISP_OK;
}

int lisp_make_cons(lisp_vm_t * vm,
                   lisp_cell_t * cell,
                   const lisp_cell_t * car,
                   const lisp_cell_t * cdr)
{
  int ret;
  cell->type_id = LISP_TID_CONS;
  cell->data.obj = lisp_gc_alloc_root_cons(&vm->gc);
  ret = _lisp_make_cons_cell(vm,
                             &((lisp_cons_t*)cell->data.obj)->car,
                             car);
  if(!ret)
  {
    return _lisp_make_cons_cell(vm,
                                &((lisp_cons_t*)cell->data.obj)->cdr,
                                cdr);
  }
  else
  {
    return ret;
  }
}