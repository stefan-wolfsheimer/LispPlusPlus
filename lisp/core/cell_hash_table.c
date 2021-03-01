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
#include "cell.h"
#include "tid.h"
#include "error.h"
#include <lisp/util/xmalloc.h>
#include <lisp/util/hash_table.h>
#include <lisp/util/murmur_hash3.h>

#define __AS_CELL__(__PTR__) ((const lisp_cell_t*)(__PTR__))
static int _lisp_cell_eq(const void * a, const void * b)
{
  if(__AS_CELL__(a)->type_id == __AS_CELL__(b)->type_id)
  {
    switch(LISP_STORAGE_ID(__AS_CELL__(a)->type_id))
    {
    case LISP_STORAGE_NULL:
      /* two NIL are always equal*/
      return 1;

    case LISP_STORAGE_ATOM:
      /* @todo */
      return 0;

    case LISP_STORAGE_COW_OBJECT:
      /* @todo */
      return 0;

    case LISP_STORAGE_OBJECT:
      /* @todo */
      return 0;

    case LISP_STORAGE_CONS:
    case LISP_STORAGE_COMPLEX:
      return __AS_CELL__(a)->data.obj == __AS_CELL__(b)->data.obj;
    default:
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
  switch(LISP_STORAGE_ID(__AS_CELL__(a)->type_id))
  {
  case LISP_STORAGE_NULL:
    data[0] = __AS_CELL__(a)->type_id;
    murmur_hash3_x86_32(data, 1, seed, &value);
    return value;

  case LISP_STORAGE_ATOM:
    /* @todo other types */
    return 0;

  case LISP_STORAGE_COW_OBJECT:
    /* @todo other types */
    return 0;

  case LISP_STORAGE_OBJECT:
    /* @todo other types */
    return 0;

  case LISP_STORAGE_CONS:
  case LISP_STORAGE_COMPLEX:
    data[0] = __AS_CELL__(a)->type_id;
    _lisp_hash_init_data(data + 1,
                         (unsigned char*) &__AS_CELL__(a)->data.obj,
                         sizeof(void*));
    murmur_hash3_x86_32(data, sizeof(void*) + 1, seed, &value);
    return value;

  default:
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
  switch(LISP_STORAGE_ID(cell->type_id))
  {
  case LISP_STORAGE_NULL:
    /* @todo */
    return 0;
  case LISP_STORAGE_ATOM:
    /* @todo */
    return 0;
  case LISP_STORAGE_COW_OBJECT:
    /* @todo */
    return 0;
  case LISP_STORAGE_OBJECT:
  case LISP_STORAGE_CONS:
  case LISP_STORAGE_COMPLEX:
    cell->data.obj = __AS_CELL__(src)->data.obj;
    return 0;
  default:
    return 0;
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

int lisp_init_cell_hash_map(struct hash_table_t * ht,
                            size_t extra_size)
{
  int res = hash_table_init(ht,
                            _lisp_cell_eq,
                            _lisp_cell_hash_function,
                            _lisp_cell_hash_entry_constructor,
                            NULL,
                            0);
  if( res == HASH_TABLE_OK )
  {
    ht->user_data = MALLOC(sizeof(size_t));
    if(ht->user_data)
    {
      *((size_t*)ht->user_data) = extra_size;
      return LISP_OK;
    }
    else
    {
      return LISP_BAD_ALLOC;
    }
  }
  else
  {
    return LISP_BAD_ALLOC;
  }
}

void lisp_free_cell_hash_table(hash_table_t * ht)
{
  if(ht->user_data != NULL)
  {
    FREE(ht->user_data);
  }
  hash_table_finalize(ht);
}

lisp_cell_t * lisp_cell_hash_table_find_or_insert(hash_table_t * ht,
                                                  const lisp_cell_t * cell,
                                                  int * inserted)
{
  return (lisp_cell_t*)hash_table_find_or_insert(ht,
                                                 cell,
                                                 sizeof(lisp_cell_t) +
                                                 (ht->user_data ?
                                                  *((size_t*)ht->user_data) : 0),
                                                 inserted);
}

struct lisp_cell_t * lisp_cell_hash_table_find(struct hash_table_t * ht,
                                               const struct lisp_cell_t * cell)
{
  return (lisp_cell_t*)hash_table_find(ht, cell);
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
