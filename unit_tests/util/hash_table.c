#include <lisp/util/dl_list.h>
#include <lisp/util/unit_test.h>
#include <lisp/util/xmalloc.h>
#include <lisp/util/hash_table.h>
#include <string.h>

#define ASSERT_HT_HAS_ELEMENTS(__TEST__, __HT__, __ELEMENTS__, __NELEM__) \
  if(!unit_add_assertion((__TEST__),                                    \
                         create_assertion_ht_has_elements( (__TEST__),  \
                                                           (__HT__),    \
                                                           #__HT__,     \
                                                           #__ELEMENTS__, \
                                                           (const char**) \
                                                           (__ELEMENTS__), \
                                                           (__NELEM__), \
                                                           __FILE__,    \
                                                           __LINE__)))  \
                                                           { return; }

#define ASSERT_HT_EQ_PTR(__TEST__, __LHS__, __RHS__, __AFILE__, __ALINE__) \
  unit_add_assertion((__TEST__),                                        \
                     assertion_create_cmp_ptr((__AFILE__),(__ALINE__),  \
                                              #__LHS__, #__RHS__,       \
                                              (__LHS__), (__RHS__),     \
                                              "==", 1))

#define ASSERT_HT_NEQ_PTR(__TEST__, __LHS__, __RHS__, __AFILE__, __ALINE__) \
  unit_add_assertion((__TEST__),                                        \
                     assertion_create_cmp_ptr((__AFILE__),(__ALINE__),  \
                                              #__LHS__, #__RHS__,       \
                                              (__LHS__), (__RHS__),     \
                                              "!=", 1))

static int my_strcmp(const void * a, const void * b) 
{
  return strcmp( *((const char**)a), *((const char**)b));
}

static int ht_cmp_function(const void * _a, const void * _b)
{
  /* @todo use non trivial cases with "keystr\tvaluestr" */
  const char * a = (const char *)_a;
  const char * b = (const char *)_b;
  size_t la = 0;
  size_t lb = 0;
  size_t len = 0;
  int    cmp;
  while(*a) 
  {
    if(*a == '\t')
    {
      break;
    }
    la++;
    a++;
  }
  while(*b) 
  {
    if(*b == '\t')
    {
      break;
    }
    lb++;
    b++;
  }
  if(la < lb) len = la;
  else        len = lb;
  a = (const char *)_a;
  b = (const char *)_b;
  cmp = strncmp(a,b,len);
  if(cmp == 0) 
  {
    if(la == lb) 
    {
      return 1;
    }
    else 
    {
      return 0;
    }
  }
  else 
  {
    return 0;
  }
}

static hash_code_t ht_hash_function(const void * a)
{
  return atoi((const char*) a);
}

static int ht_constructor(void * target, 
                          const void * src, 
                          size_t size,
                          void * user_data)
{
  memcheck_register_alloc( __FILE__, __LINE__, target);
  strcpy((char*)target, (const char*)src);
  return 0;
}

static void ht_destructor(void * what, void * user_data)
{
  memcheck_register_freed(  __FILE__, __LINE__, what);
}

static char** ht_create_n_elements(size_t n) 
{
  char ** elements = MALLOC(sizeof(char*)*n);
  size_t i;
  for(i = 0; i < n; i++) 
  {
    elements[i] = MALLOC(sizeof(char) * 10);
    sprintf(elements[i], "%zu", i);
  }
  return elements;
}

static char** ht_init_n_elements(hash_table_t * ht, size_t n)
{
  size_t i;
  char ** elements = ht_create_n_elements(n);
  hash_table_init(ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  10);
  ht->autoswap = 0;
  for(i = 0; i < n; i++) 
  {
    hash_table_set(ht, elements[i], strlen(elements[i])+1);
  }
  return elements;
}

static void ht_free_n_elements(char ** elements, size_t n)
{
  size_t i;
  for(i = 0; i < n; i++) 
  {
    FREE(elements[i]);
  }
  FREE(elements);
}

static const char ** _ht_sort_values(const char ** elements, 
                                     size_t n0, 
                                     size_t * n) 
{
  size_t i, n1;
  n1 = 0;
  for(i = 0; i < n0; i++) 
  {
    if(elements[i])
    {  
      n1++;
    }
  }
  const char ** required_values = MALLOC(sizeof(char*)*n1);
  n1 = 0;
  for(i = 0; i < n0; i++) 
  {
    if(elements[i])
    {
      required_values[n1++] = elements[i];
    }
  }
  qsort (required_values, n1, sizeof(char*), my_strcmp);
  *n = n1;
  return required_values;
}

static int _ht_check_bucket_entries(unit_test_t       * tst,
                                    hash_table_t      * ht,
                                    const char        * file,
                                    int                 line) 
{
  /** buckets have at least one entry */
  hash_table_bucket_t * bucket  = ht->first;
  hash_table_bucket_t * prev    = NULL;
  int linked_list_ok = 1;
  size_t dist;
  short int index = 1-ht->current_world_index;
  while(bucket) 
  {
    if(bucket == ht->first_new_world) 
    {
      index = ht->current_world_index;
    }
    dist = bucket - ht->hash_array[index].buckets;
    if(!CHECK_GT_U(tst, ht->hash_array[index].n_buckets, dist))
    {
      linked_list_ok = 0;
    }
    /* buckets contain at least one element */
    if(!ASSERT_HT_NEQ_PTR(tst, 
                         bucket->first,
                         NULL, 
                         file,line)) 
      linked_list_ok = 0;
    if(!ASSERT_HT_NEQ_PTR(tst, 
                          bucket->last,
                          NULL, 
                          file,line)) 
      linked_list_ok = 0;
    if(!ASSERT_HT_EQ_PTR(tst, 
                         bucket->prev,
                         prev, 
                         file,line)) 
      linked_list_ok = 0;
    if(bucket == ht->last) 
    {
      /* last */
      if(!ASSERT_HT_EQ_PTR(tst, 
                           bucket->next,
                           NULL,
                           file, line)) 
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst, 
                           bucket->last->next,
                           NULL,
                           file, line)) 
        linked_list_ok = 0;
    }
    else 
    {
      /* not last */
      if(!ASSERT_HT_NEQ_PTR(tst, 
                            bucket->next,
                            NULL,
                            file, line)) 
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst, 
                           bucket->next->prev,
                           bucket,
                           file, line)) 
        linked_list_ok = 0;
      if(!ASSERT_HT_NEQ_PTR(tst,
                            bucket->next->first,
                            NULL,
                            file, line))
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst,
                           bucket->next->first->prev,
                           bucket->last,
                           file, line))
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst,
                           bucket->last->next,
                           bucket->next->first,
                           file, line))
        linked_list_ok = 0;
    }
    if(bucket == ht->first)
    {
      /* first */
      if(!ASSERT_HT_EQ_PTR(tst, 
                           bucket->prev,
                           NULL,
                           file, line)) 
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst, 
                           bucket->first->prev,
                           NULL,
                           file, line)) 
        linked_list_ok = 0;
    }
    else 
    {
      /* not first */
      if(!ASSERT_HT_NEQ_PTR(tst, 
                            bucket->prev,
                            NULL,
                            file, line)) 
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst, 
                           bucket->prev->next,
                           bucket,
                           file, line)) 
        linked_list_ok = 0;
      if(!ASSERT_HT_NEQ_PTR(tst,
                            bucket->prev->last,
                            NULL,
                            file, line))
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst,
                           bucket->prev->last->next,
                           bucket->first,
                           file, line))
        linked_list_ok = 0;
      if(!ASSERT_HT_EQ_PTR(tst,
                           bucket->first->prev,
                           bucket->prev->last,
                           file, line))
        linked_list_ok = 0;
    }
    prev = bucket;
    bucket = bucket->next;
  }
  return linked_list_ok;
}


static assertion_t* 
create_assertion_ht_has_elements(unit_test_t       * tst,
                                 hash_table_t      * ht, 
                                 const char        * ht_expression,
                                 const char        * elements_expression,
                                 const char       ** elements,
                                 size_t              n0,
                                 const char        * file,
                                 int                 line)
{
  size_t i = 0;
  size_t n = 0;
  int linked_list_ok = 1;
  const char ** required_values =  _ht_sort_values(elements,n0, &n);
  linked_list_ok&= _ht_check_bucket_entries(tst, ht, file, line);
  
  size_t m                      = HASH_TABLE_SIZE(ht);
  const char ** ht_values       = MALLOC(sizeof(char*)*m);
  hash_table_entry_t * entry    = HASH_TABLE_FIRST(ht);
  hash_table_entry_t * prev     = NULL;

  if(HASH_TABLE_FIRST(ht))
  {
    if(!ASSERT_HT_EQ_PTR(tst, 
                         HASH_TABLE_FIRST(ht)->prev, 
                         NULL, 
                         file,line)) 
      linked_list_ok = 0;
  }
  if(HASH_TABLE_LAST(ht))
  {
    if(!ASSERT_HT_EQ_PTR(tst, 
                         HASH_TABLE_LAST(ht)->next, 
                         NULL, 
                         file, line)) 
      linked_list_ok = 0;
  }
  i = 0;
  while(entry)
  {
    if(!ASSERT_HT_EQ_PTR(tst,
                         &entry[1],
                         hash_table_find(ht, &entry[1]),
                         file, line))
    {
      linked_list_ok = 0;
    }

    if(!ASSERT_HT_EQ_PTR(tst, 
                         entry->prev,
                         prev,
                         file, 
                         line))
    {
      linked_list_ok = 0;
    }
    if(entry->prev && entry->prev->next != entry) 
    {
      linked_list_ok = 0;
      unit_add_assertion(tst, assertion_create_cmp_ptr(file, 
                                                       line, 
                                                       "entry->prev->next",
                                                       "entry",
                                                       entry->prev->next,
                                                       entry,
                                                       "==",
                                                       1));
    }
    else if(!entry->prev && entry != HASH_TABLE_FIRST(ht))
    {
      linked_list_ok = 0;
      unit_add_assertion(tst,  assertion_create_cmp_ptr(file, 
                                                        line, 
                                                        "entry",
                                                        "HASH_TABLE_FIRST(ht)",
                                                        entry,
                                                        HASH_TABLE_FIRST(ht),
                                                        "==",
                                                        0));
    }
    if(entry->next && entry->next->prev != entry) 
    {
      linked_list_ok = 0;
      unit_add_assertion(tst,  assertion_create_cmp_ptr(file, 
                                                        line, 
                                                        "entry->next->prev",
                                                        "entry",
                                                        entry->next->prev,
                                                        entry,
                                                        "==",
                                                        0));
    } 
    else if(!entry->next && entry != HASH_TABLE_LAST(ht))
    {
      linked_list_ok = 0;
      unit_add_assertion(tst,  assertion_create_cmp_ptr(file, 
                                                        line, 
                                                        "entry",
                                                        "HASH_TABLE_LAST(ht)",
                                                        entry,
                                                        HASH_TABLE_LAST(ht),
                                                        "==",
                                                        0));
    }
    ht_values[i++] = HASH_TABLE_DATA(entry, const char);
    prev = entry;
    entry = HASH_TABLE_NEXT(entry);
  }
  qsort (ht_values, m, sizeof(char*), my_strcmp);
  assertion_t * assertion;
  assertion = assertion_create_cmp_arr_cstr(file, 
                                            line, 
                                            ht_expression,
                                            elements_expression,
                                            ht_values,
                                            m,
                                            required_values,
                                            n,
                                            "==",
                                            0);
  FREE(ht_values);
  FREE(required_values);
  return assertion;
}

static void test_hash_function(unit_test_t * tst)
{
  ASSERT_EQ_U(tst, ht_hash_function("123"),      123);
  ASSERT_EQ_U(tst, ht_hash_function("123\tdef"), 123);
}

static void test_cmp_function(unit_test_t * tst)
{
  ASSERT(tst,       ht_cmp_function("123","123"));
  ASSERT_FALSE(tst, ht_cmp_function("123", "1234"));
  ASSERT(tst,       ht_cmp_function("123\tabc", "123\tdef"));
}

static void test_hash_table_init(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t ht;
  hash_table_init(&ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  1);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_buckets, 1);
  hash_table_finalize(&ht);

  hash_table_init(&ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  0);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_buckets, 1);
  hash_table_finalize(&ht);

  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_init_failure(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t ht;
  memcheck_expected_alloc(0);
  ASSERT(tst, hash_table_init(&ht, 
                              ht_cmp_function,
                              ht_hash_function,
                              ht_constructor,
                              ht_destructor,
                              1));
  memcheck_expected_alloc(1);
  memcheck_expected_alloc(0);
  ASSERT(tst, hash_table_init(&ht, 
                              ht_cmp_function,
                              ht_hash_function,
                              ht_constructor,
                              ht_destructor,
                              1));
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_hash_table_overwrite_value(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t   ht;
  int            inserted;
  const char * pair1 = "123\tabc";
  const char * pair2 = "123\tdefghi";
  const char * res1;
  const char * res2;
  hash_table_init(&ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  10);
  res1 = hash_table_set(&ht, pair1, strlen(pair1)+1);
  ASSERT_EQ_CSTR(tst, res1, pair1);
  inserted = 1;
  ASSERT_EQ_CSTR(tst, hash_table_find_or_insert(&ht, 
                                                pair2,
                                                strlen(pair2)+1,
                                                &inserted),
                 pair1);
  ASSERT_FALSE(tst, inserted);
  res2 = hash_table_set(&ht, pair2, strlen(pair2)+1);
  ASSERT_EQ_CSTR(tst, res2, pair2);

  hash_table_finalize(&ht);
  ht.autoswap = 0;
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_set(unit_test_t * tst,
                                const char  * elements[],
                                size_t        n)
{
  size_t         i,j;
  hash_table_t   ht;
  memcheck_begin();
  hash_table_init(&ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  10);
  ht.autoswap = 0;
  char **        elements_in_table = MALLOC(sizeof(char*)*n);
  for(i=0; i < n; i++)
  {
    elements_in_table[i] = NULL;
  }
  for(i = 0; i < n; i++)
  {
    elements_in_table[i] = hash_table_set(&ht, elements[i], 
                                          strlen(elements[i])+1);
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, i+1);
    if(!i) 
    {
      ASSERT_EQ_PTR(tst, HASH_TABLE_DATA(HASH_TABLE_FIRST(&ht), char), 
                    elements_in_table[0]);
      ASSERT_EQ_PTR(tst, HASH_TABLE_DATA(HASH_TABLE_LAST(&ht), char),  
                    elements_in_table[0]);
    }
    for(j = 0; j < n; j++) 
    {
      ASSERT_EQ_PTR(tst, hash_table_find(&ht,elements[j]), 
                    elements_in_table[j]);
    }
  }

  /* overwrite */
  for(i = n; i > 0; )
  {
    i--;
    char * tmp = elements_in_table[i];
    ASSERT_EQ_PTR(tst, hash_table_find(&ht,elements[i]), tmp);
    elements_in_table[i] = hash_table_set(&ht, elements[i], 
                                          strlen(elements[i])+1);
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);    
  }
  FREE(elements_in_table);
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_hash_table_find_or_insert(unit_test_t * tst,
                                           const char  * elements[],
                                           size_t        n)
{
  memcheck_begin();
  size_t i,j;
  hash_table_t ht;
  int inserted;
  char ** elements_in_table = MALLOC(sizeof(char*)*n);
  for(i=0; i < n; i++) 
  {
    elements_in_table[i] = NULL;
  }
  hash_table_init(&ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  10);
  ht.autoswap = 0;
  for(i = 0; i < n; i++) 
  {
    inserted = 0;
    elements_in_table[i] = 
      hash_table_find_or_insert(&ht, 
                                elements[i], 
                                strlen(elements[i])+1,
                                &inserted);
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, i+1);
    ASSERT(tst, inserted);
    for(j = 0; j < n; j++) 
    {
      ASSERT_EQ_PTR(tst, hash_table_find(&ht, elements[j]), 
                    elements_in_table[j]);
    }
  }
  /* find */
  for(i = n; i > 0; )
  {
    i--;
    inserted = 1;
    ASSERT_EQ_PTR(tst, hash_table_find_or_insert(&ht,
                                                 elements[i],
                                                 strlen(elements[i])+1,
                                                 &inserted), 
                  elements_in_table[i]);
    ASSERT_FALSE(tst, inserted);
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);    
  }
  hash_table_finalize(&ht);
  FREE(elements_in_table);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_set_one_bucket(unit_test_t * tst)
{
  size_t n = 10;
  const char * elements[] = { 
    "100", "110", "10120", "11130", "140", 
    "150", "160", "170", "180", "190" };

  test_hash_table_set(tst, elements, n);
}

static void test_hash_table_find_or_insert_one_bucket(unit_test_t * tst)
{
  size_t n = 10;
  const char * elements[] = { 
    "100", "110", "10120", "11130", "140", 
    "150", "160", "170", "180", "190" };
  test_hash_table_find_or_insert(tst, elements, n);
}

static void test_hash_table_set_different_buckets(unit_test_t * tst)
{
  const char * elements[] = { 
    "100",   "110",   "101", "10120", "11130",
    "10122", "11132", "140", "141",   "142",   
    "150",   "160",   "172", "182",   "190" };
  size_t n = 15;
  test_hash_table_set(tst, elements, n);
}


static void test_hash_table_find_or_insert_different_buckets(unit_test_t * tst)
{
  size_t n = 15;
  const char * elements[] = { 
    "100",   "110",   "101", "10120", "11130",
    "10122", "11132", "140", "141",   "142",   
    "150",   "160",   "172", "182",   "190" };
  test_hash_table_find_or_insert(tst, elements, n);
}

static void test_hash_table_set_after_swap(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t ht;
  size_t i,m,n = 100;
  char ** elements = ht_init_n_elements(&ht, n);
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  ASSERT(tst, hash_table_swap(&ht, 3));
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  /* cannot swap, old world isn't empty */
  ASSERT_FALSE(tst, hash_table_swap(&ht, 3));
  m = 0;
  for(i = 0; i < 10; i++) 
  {
    hash_table_set(&ht, elements[i], strlen(elements[i])+1);
    m+= 10;
    ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, m);
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  }
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, n);
  ht_free_n_elements(elements, n);
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_find_or_insert_after_swap(unit_test_t * tst)
{
  /* @todo */
  memcheck_begin();
  hash_table_t ht;
  int inserted;
  size_t i,m,n = 100;
  char ** elements = ht_init_n_elements(&ht, n);
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  ASSERT(tst, hash_table_swap(&ht, 3));
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  m = 0;
  for(i = 0; i < 10; i++) 
  {
    inserted = 1;
    hash_table_find_or_insert(&ht,
                              elements[i],
                              strlen(elements[i])+1,
                              &inserted);
    ASSERT_FALSE(tst, inserted);
    m+= 10;
    ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, m);
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  }
  ht_free_n_elements(elements, n);
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_remove(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t ht;
  size_t i, n = 100;
  char ** elements = ht_init_n_elements(&ht, n);
  char ** active_elements = MALLOC(sizeof(char*)*n);
  for(i = 0; i < n; i++) 
  {
    active_elements[i] = elements[i];
  }
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  ASSERT(tst, hash_table_swap(&ht, 3));
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);

  /* recycle bucket */
  ASSERT(tst, hash_table_remove(&ht, elements[0]));
  active_elements[0] = NULL;

  ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, 9);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, 90);

  /* remove first element from bucket */
  ASSERT(tst, hash_table_remove(&ht, elements[10]));
  active_elements[10] = NULL;
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, 8);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, 90);


  /* remove last element from bucket */
  ASSERT(tst, hash_table_remove(&ht, elements[90]));
  active_elements[90] = NULL;
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, 7);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, 90);

  /* remove from bulk of bucket */
  ASSERT(tst, hash_table_remove(&ht, elements[50]));
  active_elements[50] = NULL;
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, 6);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, 90);

  for(i  = 20; i < 80; i+=10) 
  {
    if(active_elements[i]) 
    {
      ASSERT(tst, hash_table_remove(&ht, elements[i]));
      active_elements[i] = NULL;
    }
    else 
    {
      ASSERT(tst, !hash_table_remove(&ht, elements[i]));
    }
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  }

  /* remove last entry in bucket */
  ASSERT(tst, hash_table_remove(&ht, elements[80]));
  active_elements[80] = NULL;
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);

  for(i  = 1; i < 100; i+=10) 
  {
    if(active_elements[i]) 
    {
      ASSERT(tst, hash_table_remove(&ht, elements[i]));
      active_elements[i] = NULL;
    }
    else 
    {
      ASSERT(tst, !hash_table_remove(&ht, elements[i]));
    }
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  }

  for(i  = 5; i < 100; i+=10) 
  {
    if(active_elements[i]) 
    {
      ASSERT(tst, hash_table_remove(&ht, elements[i]));
      active_elements[i] = NULL;
    }
    else 
    {
      ASSERT(tst, !hash_table_remove(&ht, elements[i]));
    }
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  }

  for(i  = 0; i < 100; i++) 
  {
    if(active_elements[i]) 
    {
      ASSERT(tst, hash_table_remove(&ht, elements[i]));
      active_elements[i] = NULL;
    }
    else 
    {
      ASSERT(tst, !hash_table_remove(&ht, elements[i]));
    }
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, active_elements, n);
  }
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&ht), 0);
  ht_free_n_elements(elements, n);
  FREE(active_elements);
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_recycle(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t ht;
  size_t i,m,n = 100;
  char ** elements = ht_init_n_elements(&ht, n);
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, 0);
  ASSERT_FALSE(tst, hash_table_recycle(&ht));
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, 0);

  ASSERT(tst, hash_table_swap(&ht, 3));

  m = n;
  ht.autoswap = 1;
  ASSERT(tst, hash_table_remove(&ht, elements[99]));
  ht.autoswap = 0;
  /* two buckets have been recycled */
  m-= 20;
  n--;
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  for(i = 0; i < 8; i++) 
  {
    ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, n-m);
    ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, m);
    m-= 10;
    ASSERT(tst, hash_table_recycle(&ht));
    ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  }
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_elements, 0);
  ASSERT_EQ_U(tst, ht.hash_array[1-ht.current_world_index].n_elements, n);
  ASSERT_EQ_U(tst, ht.hash_array[ht.current_world_index].n_buckets, 148);
  /* one element was removed */
  ht_free_n_elements(elements, n+1);
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_shrink_to_minimum(unit_test_t * tst)
{
  memcheck_begin();
  hash_table_t ht;
  hash_table_init(&ht, 
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  1);
  ht.autoswap = 0;
  hash_table_set(&ht, "abc", strlen("abc")+1);
  hash_table_remove(&ht, "abc");
  ht.autoswap = 1;
  ASSERT(tst, hash_table_swap(&ht, 1));
  /* @todo check if current world has min. size */
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_hash_table_clear(unit_test_t * tst)
{
  const char * elements[] = {
    "100",   "110",   "101", "10120", "11130",
    "10122", "11132", "140", "141",   "142",
    "150",   "160",   "172", "182",   "190" };
  size_t       n = 15;
  size_t       i;
  hash_table_t ht;
  memcheck_begin();
  hash_table_init(&ht,
                  ht_cmp_function,
                  ht_hash_function,
                  ht_constructor,
                  ht_destructor,
                  10);
  ht.autoswap = 0;
  for(i = 0; i < n; i++)
  {
    hash_table_set(&ht, elements[i], strlen(elements[i])+1);
  }
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, n);
  hash_table_clear(&ht);
  ASSERT_HT_HAS_ELEMENTS(tst, &ht, elements, 0);
  hash_table_finalize(&ht);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_hash_table(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "hash_table");
  TEST(suite, test_hash_function);
  TEST(suite, test_cmp_function);
  TEST(suite, test_hash_table_init);
  TEST(suite, test_hash_table_init_failure);

  TEST(suite, test_hash_table_overwrite_value);

  TEST(suite, test_hash_table_set_one_bucket);
  TEST(suite, test_hash_table_find_or_insert_one_bucket);

  TEST(suite, test_hash_table_set_different_buckets);
  TEST(suite, test_hash_table_find_or_insert_different_buckets);

  TEST(suite, test_hash_table_set_after_swap);
  TEST(suite, test_hash_table_find_or_insert_after_swap);

  TEST(suite, test_hash_table_remove);
  TEST(suite, test_hash_table_recycle);
  TEST(suite, test_hash_table_shrink_to_minimum);

  TEST(suite, test_hash_table_clear);
}
