#include "cell.h"
#include "tid.h"
#include "gc.h"

lisp_cell_t lisp_nil =
{
 type_id : LISP_TID_NIL
};

int lisp_is_atomic(lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_ATOM_TID(cell->type_id);
}

int lisp_is_object(lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_OBJECT_TID(cell->type_id);
}

int lisp_is_reference(lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_REFERENCE_TID(cell->type_id);
}

int lisp_is_complex(lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_COMPLEX_TID(cell->type_id);
}

int lisp_is_cons(lisp_cell_t * cell)
{
  return cell->type_id == LISP_TID_CONS;
}

size_t lisp_get_ref_count(lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].ref_count;
  }
  else
  {
    return 0;
  }
}

int lisp_is_root_cell(lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].lst->is_root;
  }
  else
  {
    return 1;
  }
}