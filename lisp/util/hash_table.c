#include "hash_table.h"
#include "xmalloc.h"
#include "assertion.h"

inline static void _remove_bucket(hash_table_t        * ht,
                                  hash_table_bucket_t * bucket)
{
  /* bucket not empty */
  REQUIRE_NEQ_PTR(bucket->first, NULL);
  REQUIRE_NEQ_PTR(bucket->last,  NULL);
  if(ht->first_new_world == bucket) 
  {
    ht->first_new_world = bucket->next;
  }
  if(bucket->prev) 
  {
    bucket->prev->last->next = bucket->last->next;
    if(bucket->last->next) 
    {
      bucket->last->next->prev = bucket->prev->last;
    }
    bucket->prev->next = bucket->next;
  }
  else 
  {
    ht->first = bucket->next;
    if(ht->first)
    {
      ht->first->prev = NULL;
    }
  }
  
  if(bucket->next) 
  {
    bucket->next->first->prev = bucket->first->prev;
    if(bucket->first->prev) 
    {
      bucket->first->prev->next = bucket->next->first;
    }
    bucket->next->prev = bucket->prev;
  }
  else 
  {
    ht->last = bucket->prev;
    if(ht->last) 
    {
      ht->last->next = NULL;
      if(ht->last->last)
      {
        ht->last->last->next = NULL;
      }
    }
  }
  bucket->first->prev = NULL;
  bucket->last->next  = NULL;
  bucket->first = NULL;
  bucket->last = NULL;
}

inline static void _add_entry_to_bucket(hash_table_t        * ht, 
                                        hash_table_bucket_t * bucket, 
                                        hash_table_entry_t  * entry) 
{
  if(bucket->last) 
  {
    /* append bucket */
    entry->prev = bucket->last;
    entry->next = bucket->last->next;
    entry->prev->next = entry;
    if(entry->next) 
    {
      entry->next->prev = entry;
    }
    bucket->last = entry;
  }
  else 
  {
    /* first entry in bucket */
    bucket->prev  = ht->last;
    bucket->next  = NULL;
    if(bucket->prev) 
    {
      bucket->prev->next = bucket;
      entry->prev        = bucket->prev->last;
      entry->prev->next  = entry;
      ht->last           = bucket;
    }
    else 
    {
      /* first bucket */
      ht->first   = bucket;
      ht->last    = bucket;
      entry->prev = NULL;
    }
    if(!ht->first_new_world) 
    {
      ht->first_new_world = bucket;
    }
    entry->next   = NULL;
    bucket->first = entry;
    bucket->last  = entry;
  }
}

inline static void _remove_entry(hash_table_t        * ht, 
                                 hash_table_bucket_t * bucket,
                                 hash_table_entry_t  * entry)
{
  /* assumption: entry is member of bucket */
  if(bucket->first == bucket->last)
  {
    /* remove one and only from bucket */
    /* assertion: entry == bucket->first */
    _remove_bucket(ht, bucket);
    return;
  }
  else if(entry == bucket->first) 
  {
    /* bucket contains more than one entry */
    /* assertion: entry->next != NULL */
    if(bucket->prev) 
    {
      bucket->prev->last->next = entry->next;
      entry->next->prev = bucket->prev->last;
    }
    else 
    {
      entry->next->prev = NULL;
    }
    bucket->first = entry->next;
  }
  else if(entry == bucket->last) 
  {
    /* assertion: entry->prev != NULL */
    if(bucket->next) 
    {
      bucket->next->first->prev = entry->prev;
      entry->prev->next = bucket->next->first;
    }
    else 
    {
      entry->prev->next = NULL;
    }
    bucket->last = entry->prev;
  }
  else 
  {
    /* assert entry->prev, entry->next */
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
  }
}

inline static void ht_bucket_list_append(hash_table_t        * ht, 
                                         hash_table_bucket_t * bucket,
                                         hash_table_entry_t  * entry)
{
  entry->prev = bucket->last;
  entry->next = NULL;
  
  if(entry->prev) 
  {
    entry->prev->next = entry;
  }
  else 
  {
    bucket->first = entry;
    bucket->prev = ht->last;
    bucket->next = NULL;
    if(bucket->prev)
    {
      bucket->prev->next = bucket;
    }
    else 
    {
      ht->first = bucket;
    }
    ht->last = bucket;
  }
  bucket->last = entry;
}

int hash_table_init(hash_table_t              * ht,
                    hash_table_eq_function_t    eq_func,
                    hash_table_hash_function_t  hash_func,
                    hash_table_constructor_t    constructor,
                    hash_table_destructor_t     destructor,
                    size_t                      min_size)
{
  if(min_size < 1) 
  {
    min_size = 1;
  }
  hash_table_bucket_t * bucket1;
  hash_table_bucket_t * bucket2;
  
  bucket1 = MALLOC(sizeof(hash_table_bucket_t) * min_size);
  if(bucket1 == NULL) 
  {
    return HASH_TABLE_ALLOC_ERROR;
  }
  bucket2 = MALLOC(sizeof(hash_table_bucket_t));
  if(bucket2 == NULL) 
  {
    FREE(bucket1);
    return HASH_TABLE_ALLOC_ERROR;
  }
  ht->eq_function              = eq_func;
  ht->hash_function            = hash_func;
  ht->constructor              = constructor;
  ht->destructor               = destructor;
  ht->user_data                = NULL;
  ht->min_bucket_size          = min_size;
  ht->current_world_index      = 0;
  ht->autoswap                 = 1;
  ht->first                    = NULL;
  ht->last                     = NULL;
  ht->first_new_world          = NULL;
  ht->hash_array[0].n_elements = 0;
  ht->hash_array[1].n_elements = 0;
  ht->hash_array[0].n_buckets  = min_size;
  ht->hash_array[1].n_buckets  = 1;
  ht->hash_array[0].buckets    = bucket1;
  ht->hash_array[1].buckets    = bucket2;
  
  size_t i;
  for(i = 0; i < min_size; i++) 
  {
    ht->hash_array[0].buckets[i].first = NULL;
    ht->hash_array[0].buckets[i].last = NULL;
    ht->hash_array[0].buckets[i].next = NULL;
    ht->hash_array[0].buckets[i].prev = NULL;
  }
  ht->hash_array[1].buckets[0].first = NULL;
  ht->hash_array[1].buckets[0].last  = NULL;
  ht->hash_array[1].buckets[0].next  = NULL;
  ht->hash_array[1].buckets[0].prev  = NULL;

  ht->resize_factor = 1.5;
  ht->lower_occ     = 0.5;
  ht->upper_occ     = 2.0;
  return HASH_TABLE_OK;
}

void hash_table_finalize(hash_table_t * ht)
{
  if(ht->first) 
  {
    hash_table_entry_t * curr = ht->first->first;
    while(curr)
    {
      hash_table_entry_t * next = curr->next;
      if(ht->destructor) 
      {
        ht->destructor(HASH_TABLE_DATA(curr, void), ht->user_data);
      }
      FREE(curr);
      curr = next;
    }
  }
  if(ht->hash_array[0].buckets) 
  {
    FREE(ht->hash_array[0].buckets);
  }
  if(ht->hash_array[1].buckets) 
  {
    FREE(ht->hash_array[1].buckets);
  }
}

void * hash_table_find_func(hash_table_t            * ht, 
                            const void              * what,
                            hash_code_t               code,
                            hash_table_eq_function_t  eq_func)
{
  hash_table_entry_t * entry = HASH_TABLE_FIRST(ht);
  while(entry != NULL) 
  {
    if(eq_func(HASH_TABLE_DATA(entry, void), what)) 
    {
      return HASH_TABLE_DATA(entry, void);
    }
    entry = entry->next;
  }
  return NULL;
}

void * hash_table_find(hash_table_t * ht, 
                       const void * what)
{
  return hash_table_find_func(ht, 
                              what, 
                              ht->hash_function(what),
                              ht->eq_function);
}


void * hash_table_find_or_insert_func(hash_table_t            * ht, 
                                      const void              * what, 
                                      size_t                    size_required,
                                      hash_code_t               code,
                                      hash_table_eq_function_t  eq_func,
                                      int                     * inserted)
{
  hash_table_bucket_t * bucket, * new_bucket;
  hash_table_entry_t  * entry, *next;
  void * found = NULL;
  /* recycle old world */
  bucket = &ht->hash_array[1-ht->current_world_index]
    .buckets[code % ht->hash_array[1-ht->current_world_index].n_buckets];
  if(ht->autoswap && bucket != ht->first)
  {
    hash_table_recycle(ht);
  }
  entry = bucket->first;
  if(entry) 
  {
    _remove_bucket(ht, bucket);
    while(entry) 
    {
      new_bucket = &ht->hash_array[ht->current_world_index]
        .buckets[entry->hash_code % ht->hash_array[ht->current_world_index].n_buckets];
      next = entry->next;
      _add_entry_to_bucket(ht, new_bucket, entry);
      ht->hash_array[1-ht->current_world_index].n_elements--;
      ht->hash_array[  ht->current_world_index].n_elements++;
      if(eq_func(HASH_TABLE_DATA(entry, void), what)) 
      {
        found = HASH_TABLE_DATA(entry, void);
      }
      entry = next;
    }
    bucket->first = NULL;
    bucket->last = NULL;
  }
  if(found) 
  {
    *inserted = 0;
    return found;
  }
  /* find in new world */
  bucket = &ht->hash_array[ht->current_world_index]
    .buckets[code % ht->hash_array[ht->current_world_index].n_buckets];
  /* find entry in bucket */
  entry = bucket->first;
  if(entry) 
  {
    while(1) 
    {
      if(eq_func(HASH_TABLE_DATA(entry, void), what)) 
      {
        *inserted = 0;
        return HASH_TABLE_DATA(entry, void);
      }
      if(entry == bucket->last) 
      {
        break;
      }
      entry = entry->next;
    }
  }

  /* not found in bucket -> create new entry in bucket */
  entry = MALLOC(sizeof(hash_table_entry_t) + size_required);
  if(entry == NULL) 
  {
    return entry;
  }
  entry->size      = size_required;
  entry->hash_code = code;
  _add_entry_to_bucket(ht, bucket, entry);
  ht->constructor((void*)&entry[1],
                  what,
                  size_required,
                  ht->user_data);
  ht->hash_array[ht->current_world_index].n_elements++;
  *inserted = 1;
  return &entry[1];
}


void * hash_table_find_or_insert(hash_table_t  * ht, 
                                 const void    * what, 
                                 size_t          size_required,
                                 int           * inserted)
{
  ht->hash_function(what);
  return hash_table_find_or_insert_func(ht, 
                                        what,
                                        size_required, 
                                        ht->hash_function(what),
                                        ht->eq_function,
                                        inserted);
}

int hash_table_remove_func(hash_table_t            * ht, 
                           const void              * what,
                           hash_code_t               code,
                           hash_table_eq_function_t  eq_func)
{
  hash_table_bucket_t * bucket, * new_bucket;
  hash_table_entry_t  * entry, *next;
  int found = 0;

  /* recycle old world */
  bucket = &ht->hash_array[1-ht->current_world_index]
    .buckets[code % ht->hash_array[1-ht->current_world_index].n_buckets];
  if(ht->autoswap && bucket != ht->first)
  {
    hash_table_recycle(ht);
  }
  entry = bucket->first;
  if(entry) 
  {
    _remove_bucket(ht, bucket);
    while(entry) 
    {
      new_bucket = &ht->hash_array[ht->current_world_index]
        .buckets[entry->hash_code % ht->hash_array[ht->current_world_index].n_buckets];
      next = entry->next;
      ht->hash_array[1-ht->current_world_index].n_elements--;
      if(!found && eq_func(HASH_TABLE_DATA(entry, void), what)) 
      {
        if(ht->destructor) 
        {
          ht->destructor(HASH_TABLE_DATA(entry, void), ht->user_data);
        }
        FREE(entry);
        found = 1;
      }
      else 
      {
        _add_entry_to_bucket(ht, new_bucket, entry);
        ht->hash_array[  ht->current_world_index].n_elements++;
      }
      entry = next;
    }
    bucket->first = NULL;
    bucket->last = NULL;
  }
  if(found) 
  {
    return 1;
  }


  bucket = &ht->hash_array[ht->current_world_index]
    .buckets[code % ht->hash_array[ht->current_world_index].n_buckets];
  /* find entry in bucket */
  entry = bucket->first;
  if(entry) 
  {
    while(1) 
    {
      if(eq_func(HASH_TABLE_DATA(entry, void), what)) 
      {
        if(ht->destructor) 
        {
          ht->destructor(HASH_TABLE_DATA(entry, void), ht->user_data);
        }
        _remove_entry(ht, bucket, entry);
        ht->hash_array[ht->current_world_index].n_elements--;        
        FREE(entry);
        return 1;
      }
      if(entry == bucket->last) 
      {
        break;
      }
      else 
      {
        entry = entry->next;
      }
    }
  }
  return 0;
}

int hash_table_remove(hash_table_t  * ht, 
                      const void    * what)
{
  return hash_table_remove_func(ht, 
                                what, 
                                ht->hash_function(what),
                                ht->eq_function);
}


void * hash_table_set_func(hash_table_t            * ht, 
                           const void              * what, 
                           size_t                    size_required,
                           hash_code_t               code,
                           hash_table_eq_function_t  eq_func)
{
  hash_table_bucket_t * bucket, * new_bucket;
  hash_table_entry_t  * entry, *next;
  /* recycle old world */
  bucket = &ht->hash_array[1-ht->current_world_index]
    .buckets[code % ht->hash_array[1-ht->current_world_index].n_buckets];
  if(ht->autoswap && bucket != ht->first)
  {
    hash_table_recycle(ht);
  }
  entry = bucket->first;
  if(entry) 
  {
    _remove_bucket(ht, bucket);
    while(entry) 
    {
      new_bucket = &ht->hash_array[ht->current_world_index]
        .buckets[entry->hash_code % ht->hash_array[ht->current_world_index].n_buckets];
      next = entry->next;
      _add_entry_to_bucket(ht, new_bucket, entry);
      ht->hash_array[1-ht->current_world_index].n_elements--;
      ht->hash_array[ht->current_world_index].n_elements++;
      entry = next;
    }
    bucket->first = NULL;
    bucket->last = NULL;
  }

  bucket = &ht->hash_array[ht->current_world_index]
    .buckets[code % ht->hash_array[ht->current_world_index].n_buckets];
  /* find entry in bucket */
  entry = bucket->first;
  if(entry) 
  {
    while(1) 
    {
      if(eq_func(HASH_TABLE_DATA(entry, void), what)) 
      {
        if(entry->size == size_required) 
        {
          if(ht->destructor) 
          {
            ht->destructor(HASH_TABLE_DATA(entry, void), ht->user_data);
          }
          entry->hash_code = code;
          ht->constructor((void*)&entry[1],
                          what,
                          size_required,
                          ht->user_data);
          return entry;
        }
        else 
        {
          hash_table_entry_t  * new_entry;
          new_entry = MALLOC(sizeof(hash_table_entry_t) + size_required);
          if(entry->prev)            entry->prev->next = new_entry;
          if(entry->next)            entry->next->prev = new_entry;
          if(entry == bucket->first) bucket->first     = new_entry;
          if(entry == bucket->last)  bucket->last      = new_entry;
          new_entry->prev      = entry->prev;
          new_entry->next      = entry->next;
          new_entry->size      = size_required;
          new_entry->hash_code = code;
          if(ht->destructor) 
          {
            ht->destructor(HASH_TABLE_DATA(entry, void), ht->user_data);
          }
          FREE(entry);
          ht->constructor((void*)&new_entry[1],
                          what,
                          size_required,
                          ht->user_data);
          return &new_entry[1];
        }
      }
      if(entry == bucket->last) 
      {
        break;
      }
      entry = entry->next;
    }
  }
  /* not found in bucket -> create new entry in bucket */
  entry = MALLOC(sizeof(hash_table_entry_t) + size_required);
  entry->size = size_required;
  entry->hash_code = code;
  _add_entry_to_bucket(ht, bucket, entry);
  ht->constructor((void*)&entry[1],
                  what,
                  size_required,
                  ht->user_data);
  ht->hash_array[ht->current_world_index].n_elements++;
  return &entry[1];
}

void * hash_table_set(hash_table_t * ht, 
                      const void   * what, 
                      size_t         size_required)
{
  return hash_table_set_func(ht, 
                             what, 
                             size_required, 
                             ht->hash_function(what),
                             ht->eq_function);
}


int hash_table_clear(hash_table_t * ht)
{
  hash_table_finalize(ht);
  return hash_table_init(ht,
                         ht->eq_function,
                         ht->hash_function,
                         ht->constructor,
                         ht->destructor,
                         ht->min_bucket_size);
}

int hash_table_swap(hash_table_t * ht, 
                    size_t         n)
{
  if(ht->hash_array[1-ht->current_world_index].n_elements) 
  {
    return 0;
  }
  else 
  {
    hash_table_bucket_t * buckets;
    size_t i;
    ht->current_world_index = 1-ht->current_world_index;
    buckets = ht->hash_array[ht->current_world_index].buckets;
    buckets = REALLOC(buckets, (sizeof(hash_table_bucket_t) * n));
    for(i = 0; i < n; i++) 
    {
      buckets[i].first = NULL;
      buckets[i].last = NULL;
      buckets[i].next = NULL;
      buckets[i].prev = NULL;
    }
    ht->hash_array[ht->current_world_index].buckets = buckets;
    ht->hash_array[ht->current_world_index].n_buckets  = n;
    ht->first_new_world = NULL;
    return 1;
  }
}

int hash_table_recycle(hash_table_t * ht)
{
  if(ht->first && ht->first != ht->first_new_world) 
  {
    hash_table_entry_t  * entry, *next;
    hash_table_bucket_t * new_bucket, * bucket;
    bucket = ht->first;
    REQUIRE_NEQ_PTR(bucket->first, NULL);
    entry = bucket->first;
    _remove_bucket(ht, bucket);
    while(entry) 
    {
      new_bucket = &ht->hash_array[ht->current_world_index]
        .buckets[entry->hash_code % 
                 ht->hash_array[ht->current_world_index].n_buckets];
      next = entry->next;
      _add_entry_to_bucket(ht, new_bucket, entry);
      ht->hash_array[1-ht->current_world_index].n_elements--;
      ht->hash_array[  ht->current_world_index].n_elements++;
      entry = next;
    }
    bucket->first = NULL;
    bucket->last = NULL;
    if(ht->first == ht->first_new_world) 
    {
      REQUIRE_EQ_U(ht->hash_array[1-ht->current_world_index].n_elements, 0);
      hash_table_array_t * array = &ht->hash_array[ht->current_world_index];
      float occ = (float)array->n_elements / (float)array->n_buckets;
      if(occ < ht->lower_occ || occ > ht->upper_occ)
      {
        size_t n_new = ht->hash_array[ht->current_world_index].n_elements * ht->resize_factor;
        if(n_new < ht->min_bucket_size) 
        {
          n_new = ht->min_bucket_size;
        }
        array = &ht->hash_array[1-ht->current_world_index];
        size_t i;
        hash_table_bucket_t * buckets;
        buckets = array->buckets = REALLOC(array->buckets, sizeof(hash_table_bucket_t) * n_new);
        for(i = array->n_buckets; i < n_new; i++) 
        {
          buckets[i].first = NULL;
          buckets[i].last = NULL;
        }
        array->n_buckets = n_new;
        ht->first_new_world = NULL;
        ht->current_world_index = 1-ht->current_world_index;
      }
    }
    return 1;
  }
  else 
  {
    return 0;
  }
}
