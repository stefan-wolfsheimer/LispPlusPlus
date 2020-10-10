#include "xmalloc.h"
#include "xstring.h"
#include <stdio.h>
#include <string.h>

#define MEMCHECK_BLOCK_SIZE 256

static memchecker_t ** memcheck_stack = NULL;
static size_t          memcheck_stack_size = 0;

static memchecker_chunk_t * _memcheck_find_chunk(memchecker_t * memchecker, 
						 void * ptr);
static assertion_t  * _memcheck_create_assertion(memchecker_t * memchecker, 
						 const char   * file,
						 int            line);

static memchecker_chunk_t * _memcheck_find_chunk(memchecker_t * memchecker, 
						 void * ptr)
{
  size_t i;
  for(i = 0; i < memchecker->n_chunks; i++)
  {
    if(memchecker->chunks[i].ptr == ptr)
    {
      return &memchecker->chunks[i];
    }
  }
  return NULL;
}

static assertion_t  * _memcheck_create_assertion(memchecker_t * memchecker, 
						 const char   * file,
						 int            line)
{
  assertion_t * assertion;
  assertion = assertion_create(file, line);
  if(memchecker->first_assertion == NULL) 
  {
    memchecker->first_assertion = assertion;
  }
  else 
  {
    memchecker->last_assertion->next = assertion;
  }
  memchecker->last_assertion = assertion;  
  return assertion;
}

memchecker_t * memcheck_begin()
{
  memcheck_stack_size++;
  memcheck_stack = realloc(memcheck_stack,
			   sizeof(memchecker_t*)*memcheck_stack_size);
  memchecker_t * memchecker = malloc(sizeof(memchecker_t));
  memchecker->chunks              = NULL;
  memchecker->n_chunks            = 0;
  memchecker->first_assertion     = NULL;
  memchecker->last_assertion      = NULL;
  memchecker->next_mock           = NULL;
  memchecker->last_mock           = NULL;
  memchecker->enabled             = 1;
  memcheck_stack[memcheck_stack_size-1] = memchecker;
  return memchecker;
}

int memcheck_end()
{
  size_t         i;
  memchecker_t * memchecker = memcheck_current();
  int ret = 1;
  if(memchecker) 
  {
    memcheck_retire_mocks(memchecker);
    while(memchecker->first_assertion)
    {
      if(!memchecker->first_assertion->success)
      {
	ret = 0;
      }
      memchecker->first_assertion = assertion_free(memchecker->first_assertion);
    }
    for(i = 0; i < memchecker->n_chunks; i++)
    {
      if(memchecker->chunks[i].alloc_file != NULL)
      {
        free(memchecker->chunks[i].alloc_file);
      }
      if(memchecker->chunks[i].free_file != NULL)
      {
        free(memchecker->chunks[i].free_file);
      }
    }
    free(memchecker->chunks);
    free(memchecker);
    memcheck_stack_size--;
    if(memcheck_stack_size == 0) 
    {
      free(memcheck_stack);
      memcheck_stack = NULL;
    }
  }
  return ret;
}

memchecker_t * memcheck_current()
{
  if(memcheck_stack_size)
  {
    return memcheck_stack[memcheck_stack_size-1];
  }
  else 
  {
    return NULL;
  }
}

int memcheck_enable(int enabled)
{
  memchecker_t * memchecker = memcheck_current();
  if(memchecker) 
  {
    int ret = memchecker->enabled;
    memchecker->enabled = enabled;
    return ret;
  }
  else 
  {
    return 0;
  }
}


/*********************************************************************
 * 
 * Check functions 
 *
 *********************************************************************/
static int _memcheck_check_chunk(memchecker_t       * memchecker, 
				 memchecker_chunk_t * chunk)
{
  if(chunk->alloc_file != NULL)
  {
    if(chunk->free_file == NULL)
    {
      assertion_t  * assertion = _memcheck_create_assertion(memchecker,
							    chunk->alloc_file,
							    chunk->alloc_line);
      assertion->is_exception = 1;
      int old = memcheck_enable(0);
      assertion->expect = alloc_sprintf("segment %p still reachable", chunk->ptr);      
      memcheck_enable(old);
      return 0;
    }
  }
  return 1;
}

assertion_t * memcheck_finalize()
{
  assertion_t * ret = NULL;
  memchecker_t * memchecker = memcheck_current();
  size_t i;
  if(memchecker) 
  {
    for(i = 0; i < memchecker->n_chunks; i++)
    {
      _memcheck_check_chunk(memchecker, &memchecker->chunks[i]);
    }
    ret = memchecker->first_assertion;
    memchecker->first_assertion = NULL;
  }
  return ret;
}

assertion_t * memcheck_remove_first_assertion()
{
  assertion_t * ret = NULL;
  memchecker_t * memchecker = memcheck_current();
  if(memchecker) 
  {
    ret = memchecker->first_assertion;
    if(ret) 
    {
      memchecker->first_assertion = ret->next;
      ret->next = NULL;
    }
  }
  return ret;
}

/*********************************************************************
 * 
 * Mock functions 
 *
 *********************************************************************/
void memcheck_expected_alloc(int success)
{
  memchecker_t * memchecker = memcheck_current();
  if(memchecker) 
  {
    memchecker_alloc_mock_t * mock;
    mock = malloc(sizeof(memchecker_alloc_mock_t));
    mock->prev = memchecker->last_mock;
    mock->next = NULL;
    mock->success = success;
    if(mock->prev == NULL)
    {
      memchecker->next_mock = mock;
    }
    else 
    {
      mock->prev->next = mock;
    }
    memchecker->last_mock = mock;
  }
}

int memcheck_next_mock(const char * file, int line)
{
  int ret = 1;
  memchecker_t * memchecker = memcheck_current();
  if(memchecker && memchecker->enabled && memchecker->next_mock != NULL) 
  {
    memchecker_alloc_mock_t * next;
    ret = memchecker->next_mock->success;
    next = memchecker->next_mock->next;
    free(memchecker->next_mock);
    memchecker->next_mock = next;
    if(next) 
    {
      next->prev = NULL;
    }
    else 
    {
      memchecker->last_mock = NULL;
    }
  }
  return ret;
}

int memcheck_check_mocks(memchecker_t * memchecker)
{
  int ret = 1;
  memchecker_alloc_mock_t * current_mock;
  current_mock = memchecker->next_mock;
  while(current_mock != NULL) 
  {
    ret = 0;
    /* @todo create error message, if there 
       are pending mocks*/   
    current_mock = current_mock->next;
  }
  return ret;
}

size_t memcheck_retire_mocks()
{
  memchecker_t * memchecker = memcheck_current();
  size_t ret = 0;
  if(memchecker) 
  {
    memchecker_alloc_mock_t * current_mock;
    memchecker_alloc_mock_t * next_mock;
    current_mock = memchecker->next_mock;
    while(current_mock != NULL) 
    {
      ret++;
      /* @todo create error message, if there 
	 are pending mocks*/   
      next_mock = current_mock->next;
      free(current_mock);
      current_mock = next_mock;
    }
    memchecker->next_mock = NULL;
  }
  return ret;
}

/*********************************************************************
 * 
 * Allocation functions
 *
 *********************************************************************/
void * memcheck_debug_malloc(  const char     * file,
                               int              line,
                               size_t           size)
{
  void * ptr = NULL;
  if(memcheck_next_mock(file, line)) 
  {
    ptr = malloc(size);
    memcheck_register_alloc(file, line, ptr);
  }
  return ptr;
}

void * memcheck_debug_realloc( const char     * file,
                               int              line,
                               void           * ptr,
                               size_t           size)
{
  if(memcheck_next_mock(file, line)) 
  {
    if(ptr == NULL)
    {
      ptr = malloc(size);
      memcheck_register_alloc(file, line, ptr);
      return ptr;
    }
    else
    {
      void * old = ptr;
      ptr = realloc(ptr,size);
      memcheck_register_freed(file, line, old);
      memcheck_register_alloc(file, line, ptr);
      return ptr;
    }
  }
  else 
  {
    return NULL;
  }
}

void memcheck_debug_free( const char    * file,
                          int             line,
                          void          * ptr)
{
  if(memcheck_register_freed(file, line, ptr)) 
  {
    free(ptr);
  }
}

void memcheck_register_alloc( const char * file,
                              int          line,
                              void       * ptr)
{
  memchecker_t * memchecker = memcheck_current();
  if(memchecker && memchecker->enabled) 
  {
    memchecker_chunk_t * chunk;
    chunk = _memcheck_find_chunk(memchecker, ptr);
    if(chunk == NULL) 
    {
      size_t n;
      if(memchecker->n_chunks % MEMCHECK_BLOCK_SIZE == 0)
      {
	n = memchecker->n_chunks / MEMCHECK_BLOCK_SIZE;
	n++;
	memchecker->chunks = realloc(   memchecker->chunks,
					MEMCHECK_BLOCK_SIZE * n *
					sizeof(memchecker_chunk_t));
	if(memchecker->chunks == NULL) return;
      }
      chunk = &memchecker->chunks[memchecker->n_chunks];
      chunk->ptr        = ptr;
      chunk->alloc_file = NULL;
      chunk->free_file  = NULL;
      chunk->alloc_line = 0;
      chunk->free_line  = 0;
      memchecker->n_chunks++;
    }
    if(chunk->free_file != NULL)
    {
      free(chunk->free_file);
      chunk->free_file = NULL;
      chunk->free_line = 0;
    }
    if(chunk->alloc_file != NULL) 
    {
      free(chunk->alloc_file);
      chunk->alloc_file = NULL;
      chunk->alloc_line = 0;
    }
    int old = memcheck_enable(0);
    chunk->alloc_file = alloc_strcpy(file);
    memcheck_enable(old);
    chunk->alloc_line = line;
  }
}

int memcheck_register_freed(  const char * file,
			      int          line,
			      void       * ptr)
{
  memchecker_t * memchecker = memcheck_current();
  if(memchecker) 
  {
    memchecker_chunk_t * chunk = _memcheck_find_chunk(memchecker, ptr);
    if(chunk == NULL || chunk->alloc_file == NULL)
    {
      assertion_t * assertion = _memcheck_create_assertion(memchecker, 
							   file,
							   line);
      assertion->is_exception = 1;
      int old = memcheck_enable(0);
      assertion->expect = alloc_sprintf("attempt to free unmanaged %p", ptr);
      memcheck_enable(old);
      return 1;
    }
    else if(chunk->free_file != NULL) 
    {
      assertion_t * assertion = _memcheck_create_assertion(memchecker, 
							   file,
							   line);
      assertion->is_exception = 1;
      int old = memcheck_enable(0);
      assertion->expect = alloc_sprintf("double free of %p", ptr);
      memcheck_enable(old);
      return 0;
    }
    else 
    {
      chunk->free_file = malloc(strlen(file)+1);
      strcpy(chunk->free_file, file);
      chunk->free_line = line;
      return 1;
    }
  }
  return 1;
}
