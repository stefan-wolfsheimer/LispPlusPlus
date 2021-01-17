#include "cell.h"
#include "tid.h"
#include "gc.h"
#include "cons.h"
#include "error.h"
#include <lisp/util/hash_table.h>
#include <lisp/util/murmur_hash3.h>

lisp_cell_t lisp_nil =
{
 type_id : LISP_TID_NIL
};

int lisp_is_atomic(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_ATOM_TID(cell->type_id);
}

int lisp_is_object(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_OBJECT_TID(cell->type_id);
}

int lisp_is_reference(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_REFERENCE_TID(cell->type_id);
}

int lisp_is_complex(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_COMPLEX_TID(cell->type_id);
}

struct lisp_complex_object_t * lisp_as_complex_object(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return (struct lisp_complex_object_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
}

int lisp_is_cons(const lisp_cell_t * cell)
{
  return LISP_IS_STORAGE_CONS_TID(cell->type_id);
}

struct lisp_cons_t * lisp_as_cons(lisp_cell_t * cell)
{
  if(LISP_IS_CONS_TID(cell->type_id))
  {
    return (struct lisp_cons_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
}

int lisp_is_array(const lisp_cell_t * cell)
{
  return cell->type_id == LISP_TID_ARRAY;
}

struct lisp_array_t * lisp_as_array(lisp_cell_t * cell)
{
  if(cell->type_id == LISP_TID_ARRAY)
  {
    return (struct lisp_array_t *)cell->data.obj;
  }
  else
  {
    return NULL;
  }
}

size_t lisp_get_ref_count(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    return ((lisp_cons_t*)cell->data.obj)->ref_count;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].ref_count;
  }
  else
  {
    return 0;
  }
}

int lisp_is_root_cell(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    return ((lisp_cons_t*)cell->data.obj)->gc_list->is_root;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].gc_list->is_root;
  }
  else
  {
    return 1;
  }
}

lisp_gc_color_t lisp_get_cell_color(const lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    return ((lisp_cons_t*)cell->data.obj)->gc_list->color;
  }
  else if(LISP_IS_STORAGE_COMPLEX_TID(cell->type_id))
  {
    return ((lisp_complex_object_t*)cell->data.obj)[-1].gc_list->color;
  }
  else
  {
    return LISP_GC_NO_COLOR;
  }
}

int lisp_unset(lisp_cell_t * cell)
{
  if(LISP_IS_STORAGE_ATOM_TID(cell->type_id))
  {
    cell->type_id = LISP_TID_NIL;
    cell->data.obj = NULL;
  }
  else if(LISP_IS_STORAGE_CONS_TID(cell->type_id))
  {
    lisp_cons_unset((lisp_cons_t*)cell->data.obj);
    return LISP_OK;
  }
  else
  {
    /*@todo implement other types */
    return LISP_NOT_IMPLEMENTED;
  }
  return LISP_OK;
}

