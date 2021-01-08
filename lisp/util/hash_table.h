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
#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__
#include <stdlib.h>
#include <stdint.h>

#define HASH_TABLE_OK 0x00
#define HASH_TABLE_ALLOC_ERROR 0x01

struct hash_table_entry_t;

typedef uint32_t hash_code_t;
typedef int(*hash_table_eq_function_t)(const void * a,
                                       const void * b);
/**
 * Hash function type
 */
typedef hash_code_t (*hash_table_hash_function_t)(const void * a);

typedef int(*hash_table_constructor_t)(void       * target,
                                       const void * src, 
                                       size_t       size,
                                       void       * user_data);

typedef void(*hash_table_destructor_t)(void * what,
                                       void * user_data);

typedef struct hash_table_entry_t
{
  hash_code_t                  hash_code;
  size_t                       size;
  struct hash_table_entry_t  * next;
  struct hash_table_entry_t  * prev;
  struct hash_table_bucket_t * bucket;
  struct hash_table_entry_t  * bucket_next;
  struct hash_table_entry_t  * bucket_prev;
} hash_table_entry_t;

/* private structure */
typedef struct hash_table_bucket_t 
{
  hash_table_entry_t         * first;
  hash_table_entry_t         * last;
  struct hash_table_bucket_t * next;
  struct hash_table_bucket_t * prev;
} hash_table_bucket_t;

/* private structure */
typedef struct hash_table_array_t 
{
  hash_table_bucket_t * buckets;
  size_t                n_buckets;
  size_t                n_elements;
} hash_table_array_t;

typedef struct hash_table_t
{
  hash_table_eq_function_t    eq_function;
  hash_table_hash_function_t  hash_function;
  hash_table_constructor_t    constructor;
  hash_table_destructor_t     destructor;
  void                      * user_data;

  size_t                      min_bucket_size;
  float                       resize_factor;
  float                       lower_occ;
  float                       upper_occ;

  hash_table_array_t          hash_array[2];
  size_t                      current_world_index;
  hash_table_bucket_t       * first;
  hash_table_bucket_t       * last;
  hash_table_bucket_t       * first_new_world;
  short int                   autoswap;
} hash_table_t;


#define HASH_TABLE_OK 0x00
#define HASH_TABLE_ALLOC_ERROR 0x01
/** 
 * Initialize the hash table HASH_TABLE_ALLOC_ERROR otherwise
 * @return HASH_TABLE_OK on success and 
 */
int hash_table_init(hash_table_t               * ht,
                    hash_table_eq_function_t     cmp_func,
                    hash_table_hash_function_t   hash_func,
                    hash_table_constructor_t     constructor,
                    hash_table_destructor_t      destrcutor,
                    size_t                       min_size);

void hash_table_finalize(hash_table_t * ht);

void * hash_table_find(hash_table_t * ht,
                       const void   * what);

void * hash_table_find_func(hash_table_t             * ht, 
                            const void               * what,
                            hash_code_t                code,
                            hash_table_eq_function_t   eq_func);

void * hash_table_find_or_insert(hash_table_t * ht, 
                                 const void   * what, 
                                 size_t         size_required,
                                 int          * inserted);

void * hash_table_find_or_insert_func(hash_table_t             * ht,
                                      const void               * what, 
                                      size_t                     size_required,
                                      hash_code_t                code,
                                      hash_table_eq_function_t   eq_func,
                                      int                      * inserted);

void * hash_table_set(hash_table_t  * ht, 
                      const void    * what, 
                      size_t          size_required);

void * hash_table_set_func(hash_table_t             * ht, 
                           const void               * what, 
                           size_t                     size_required,
                           hash_code_t                code,
                           hash_table_eq_function_t   eq_func);

int hash_table_remove(hash_table_t * ht,
                      const void   * what);

int hash_table_remove_func(hash_table_t             * ht, 
                           const void               * what,
                           hash_code_t                code,
                           hash_table_eq_function_t   eq_func);

int hash_table_clear(hash_table_t * ht);

/** swap functions */
int hash_table_swap(hash_table_t * ht, 
                    size_t         n);

/** Todo: swap after recycle */
int hash_table_recycle(hash_table_t * ht);

#define HASH_TABLE_FIRST(__HASH_TABLE__)                                \
  ((__HASH_TABLE__)->first ?                                            \
   (__HASH_TABLE__)->first->first : NULL)                               

#define HASH_TABLE_LAST(__HASH_TABLE__)                                 \
  ((__HASH_TABLE__)->last ?                                             \
   (__HASH_TABLE__)->last->last : NULL)                               

#define HASH_TABLE_SIZE(__HASH_TABLE__)                                 \
  ((__HASH_TABLE__)->hash_array[0].n_elements +                         \
   (__HASH_TABLE__)->hash_array[1].n_elements)

#define HASH_TABLE_NEXT(__ENTRY__)       ((__ENTRY__)->next)
#define HASH_TABLE_PREV(__ENTRY__)       ((__ENTRY__)->prev)
#define HASH_TABLE_DATA(__ENTRY__, __TYPE__)  ((__TYPE__*)&((__ENTRY__)[1]))

#endif
