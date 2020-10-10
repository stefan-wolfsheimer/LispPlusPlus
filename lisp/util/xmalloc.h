/******************************************************************************
Copyright (c) 2020, Stefan Wolfsheimer

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
#ifndef __XMALLOC_H__
#define __XMALLOC_H__
/** @file xmalloc.h 
 */
#include <stdlib.h>
#include "assertion.h"
/** 
 *  A chunk of memory managed by the memory checker
 */
typedef struct memchecker_chunk_t
{
  void  * ptr;
  char  * alloc_file; /** if alloc_file != NULL and free_file == NULL
                          the chunk is still reachable */
  int     alloc_line;

  char  * free_file; /** if alloc_file != NULL and free_file != NULL
                         the chunk is freed */
  int     free_line;
} memchecker_chunk_t;

typedef struct memchecker_alloc_mock_t 
{
  int success;
  struct memchecker_alloc_mock_t * next;
  struct memchecker_alloc_mock_t * prev;
} memchecker_alloc_mock_t;

typedef struct memchecker_t
{
  memchecker_chunk_t      * chunks;
  size_t                    n_chunks;
  assertion_t             * first_assertion;
  assertion_t             * last_assertion;
  memchecker_alloc_mock_t * next_mock;
  memchecker_alloc_mock_t * last_mock;
  int                       enabled;
} memchecker_t;

#ifdef DEBUG

#define MALLOC(SIZE) memcheck_debug_malloc(__FILE__,              \
					   __LINE__,              \
					   (SIZE))
#define REALLOC(PTR,SIZE) memcheck_debug_realloc(  __FILE__, __LINE__,  \
						   (PTR),		\
						   (SIZE))
#define FREE(PTR) memcheck_debug_free(__FILE__,__LINE__,(PTR))
#else
#define MALLOC(SIZE)                malloc((SIZE))
#define REALLOC(PTR,SIZE)           realloc((PTR),(SIZE))
#define FREE(PTR)                   free((PTR))
#endif

/**
 * Begin a context of managed memory allocations.
 */
memchecker_t * memcheck_begin();

/** 
 * End a context of managed memory allocations
 */
int memcheck_end();

/** 
 * Get the current memcheck context.
 * Returns NULL is context has not been created with memcheck_begin
 * @retiurn current context or NULL
 */
memchecker_t * memcheck_current();

/** 
 * Disable or enable allocation tracking for the current context.
 * @return non zero if tracking was enabled 
 */
int memcheck_enable(int enable);



/*********************************************************************
 * 
 * Check functions 
 *
 *********************************************************************/
/** 
 * Remove assertions from current context.
 * For all chunks that haven't been freed so far an assertion is returned.
 * @return a single-linked list of assertions
 */
assertion_t * memcheck_finalize();

/** 
 * Remove the first assertion from current context.
 * @return assertion
 */
assertion_t * memcheck_remove_first_assertion();


/*********************************************************************
 * 
 * Mock functions 
 *
 *********************************************************************/
/** 
 * Register a mock for expected allocations
 */
void memcheck_expected_alloc(int success);

int memcheck_next_mock(const char * file, int line);

/** 
 * Check if all registered mocks have been consumed 
 * @return 0 if there are pendinding mocks
 */
int memcheck_check_mocks(memchecker_t * memchecker);

/** 
 *  Remove all registered mocks from the memchecker 
 *  @return number of removed mocks
 */
size_t memcheck_retire_mocks();


/*********************************************************************
 * 
 * Allocation functions
 *
 *********************************************************************/
void memcheck_register_alloc( const char * file,
                              int          line,
                              void       * ptr);

int memcheck_register_freed(  const char * file,
			      int          line,
			      void       * ptr);

void * memcheck_debug_malloc(  const char     * file,
                               int              line,
                               size_t           size);

void * memcheck_debug_realloc( const char     * file,
                               int              line,
                               void           * ptr,
                               size_t           size);

void memcheck_debug_free( const char    * file,
                          int             line,
                          void          * ptr);

#endif
