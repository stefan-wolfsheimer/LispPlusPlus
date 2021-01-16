#include "cell.h"
#include "tid.h"
#include "error.h"
#include <lisp/util/hash_table.h>
#include <lisp/util/murmur_hash3.h>

#define __AS_CELL__(__PTR__) ((const lisp_cell_t*)(__PTR__))
static int _lisp_cell_eq(const void * a, const void * b)
{
  if(__AS_CELL__(a)->type_id == __AS_CELL__(b)->type_id)
  {
    if(LISP_IS_NIL_TID(__AS_CELL__(a)->type_id))
    {
      /* two NIL are always equal*/
      return 1;
    }
    if(LISP_IS_STORAGE_COMPLEX_OR_CONS(__AS_CELL__(a)->type_id))
    {
      return __AS_CELL__(a)->data.obj == __AS_CELL__(b)->data.obj;
    }
    else
    {
      /* @todo other types */
      return 0;
    }
  }
  else
  {
    /* different types are not equal */
    return 0;
  }
}

static void _lisp_hash_init_data(unsigned char * data,
                                 const unsigned char * input,
                                 size_t n)
{
  size_t i;
  for(i = 0; i < n; i++)
  {
    data[i] = input[i];
  }
}

static hash_code_t _lisp_cell_hash_function(const void * a)
{
  static const uint32_t seed = 1;
  uint32_t value;
  unsigned char data[sizeof(lisp_cell_t)];
  if(LISP_IS_NIL_TID(__AS_CELL__(a)->type_id))
  {
    data[0] = __AS_CELL__(a)->type_id;
    murmur_hash3_x86_32(data, 1, seed, &value);
    return value;
  }
  else if(LISP_IS_STORAGE_COMPLEX_OR_CONS(__AS_CELL__(a)->type_id))
  {
    data[0] = __AS_CELL__(a)->type_id;
    _lisp_hash_init_data(data + 1,
                         (unsigned char*) &__AS_CELL__(a)->data.obj,
                         sizeof(void*));
    murmur_hash3_x86_32(data, sizeof(void*) + 1, seed, &value);
    return value;
  }
  else
  {
    /* @todo other types */
    return 0;
  }
}

static int _lisp_cell_hash_entry_constructor(void * target,
                                             const void * src,
                                             size_t size,
                                             void * user_data)
{
  lisp_cell_t * cell = (lisp_cell_t*)target;
  cell->type_id = __AS_CELL__(src)->type_id;
  if(LISP_IS_STORAGE_REFERENCE_TID(cell->type_id) ||
     LISP_IS_STORAGE_COMPLEX_OR_CONS(cell->type_id))
  {
    cell->data.obj = __AS_CELL__(src)->data.obj;
  }
  else
  {
    /* @todo other tcopy cell */
  }
  return 0;
}
#undef __AS_CELL__

int lisp_init_cell_hash_table(hash_table_t * ht)
{
  int res = hash_table_init(ht,
                            _lisp_cell_eq,
                            _lisp_cell_hash_function,
                            _lisp_cell_hash_entry_constructor,
                            NULL,
                            0);
  return ((res == HASH_TABLE_OK) ? LISP_OK : LISP_BAD_ALLOC);
}

void lisp_free_cell_hash_table(hash_table_t * ht)
{
  hash_table_finalize(ht);
}

lisp_cell_t * lisp_cell_hash_table_find_or_insert(hash_table_t * ht,
                                                  const lisp_cell_t * cell,
                                                  int * inserted)
{
  return (lisp_cell_t*)hash_table_find_or_insert(ht,
                                                 cell,
                                                 sizeof(lisp_cell_t),
                                                 inserted);
}

lisp_cell_t * lisp_cell_hash_table_set(hash_table_t * ht,
                                       const lisp_cell_t * cell)
{
  return (lisp_cell_t*)hash_table_set(ht, cell, sizeof(lisp_cell_t));
}

int lisp_cell_hash_table_remove(struct hash_table_t * ht,
                                const lisp_cell_t * cell)
{
  return hash_table_remove(ht, cell);
}

int lisp_cell_in_hash(struct hash_table_t * ht,
                      const lisp_cell_t * cell)
{
  return (hash_table_find(ht, cell) != NULL);
}
