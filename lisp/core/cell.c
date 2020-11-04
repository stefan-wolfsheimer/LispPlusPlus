#include "cell.h"
#include "tid.h"
#include "gc.h"

lisp_cell_t lisp_nil =
{
 type_id : LISP_TID_NIL
};

int lisp_is_atomic(lisp_cell_t * cell)
{
  return (cell->type_id < LISP_TID_MANAGED_ATOM_END);
}

int lisp_is_root_cell(lisp_cell_t * cell)
{
  if(cell->type_id < LISP_TID_COMPOUND_END)
  {
    return 1;
  }
  else
  {
    return ((lisp_collectible_object_t*)cell->data.cons)[-1].lst->is_root;
  }
}
