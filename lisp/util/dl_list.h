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
#ifndef __DL_LIST_H__
#define __DL_LIST_H__

#include <assert.h>
#include <stdlib.h>

/**
 * An item of a double-linked list.
 */
typedef struct lisp_dl_item_t
{
  struct lisp_dl_item_t * prev;
  struct lisp_dl_item_t * next;
} lisp_dl_item_t;

/**
 * An double linked list
 */
typedef struct lisp_dl_list_t
{
  lisp_dl_item_t * first;
  lisp_dl_item_t * last;
} lisp_dl_list_t;

/**
 * Initialize a double-linked list.
 */
static inline void lisp_init_dl_list(lisp_dl_list_t * ll);

/**
 * Return non-zero if list is empty
 */
static inline int lisp_dl_list_empty(const lisp_dl_list_t * ll);

/**
 * Return size of the list.
 * Note: linear complexity.
 */
static inline size_t lisp_dl_list_size(const lisp_dl_list_t * ll);

/**
 * Append an item at the end of a double-linked list
 */
static inline void lisp_dl_list_append(lisp_dl_list_t * ll,
                                       lisp_dl_item_t * item);

/**
 * Prepend an item at the begin of a double-linked list
 */
static inline void lisp_dl_list_prepend(lisp_dl_list_t * ll,
                                        lisp_dl_item_t * item);

/**
 * Insert an item at a given position of a double-linked list.
 */
static inline void lisp_dl_list_insert(lisp_dl_list_t * ll,
                                       lisp_dl_item_t * pos,
                                       lisp_dl_item_t * item);

/**
 * Remove an item from a double-linked list.
 */
static inline void lisp_dl_list_remove(lisp_dl_list_t * ll,
                                       lisp_dl_item_t * item);

static inline void lisp_dl_list_remove_first(lisp_dl_list_t * ll);

/***************************************************************************
 * Implementation
 *
 ***************************************************************************/
static inline void lisp_init_dl_list(lisp_dl_list_t * ll)
{
  ll->first = NULL;
  ll->last = NULL;
}

static inline int lisp_dl_list_empty(const lisp_dl_list_t * ll)
{
  return (ll->first == NULL);
}

static inline size_t lisp_dl_list_size(const lisp_dl_list_t * ll)
{
  size_t ret = 0;
  lisp_dl_item_t * itr = ll->first;
  while(itr != NULL)
  {
    ret++;
    itr = itr->next;
  }
  return ret;
}

static inline void lisp_dl_list_append(lisp_dl_list_t * ll,
                                       lisp_dl_item_t * item)
{
  if(ll->first == NULL)
  {
    assert(ll->last == NULL);
    item->prev = NULL;
    item->next = NULL;
    ll->first = item;
    ll->last = item;
  }
  else
  {
    assert(ll->last != NULL);
    assert(ll->last->next == NULL);
    item->prev = ll->last;
    item->next = NULL;
    ll->last->next = item;
    ll->last = item;
  }
}

static inline void lisp_dl_list_prepend(lisp_dl_list_t * ll,
                                        lisp_dl_item_t * item)
{
  if(ll->first == NULL)
  {
    assert(ll->last == NULL);
    item->prev = NULL;
    item->next = NULL;
    ll->first = item;
    ll->last = item;
  }
  else
  {
    assert(ll->last != NULL);
    assert(ll->last->next == NULL);
    item->prev = NULL;
    item->next = ll->first;
    ll->first->prev = item;
    ll->first = item;
  }
}

static inline void lisp_dl_list_insert(lisp_dl_list_t * ll,
                                       lisp_dl_item_t * pos,
                                       lisp_dl_item_t * item)
{
  if(pos == NULL)
  {
    lisp_dl_list_prepend(ll, item);
  }
  else
  {
    item->prev = pos;
    item->next = pos->next;
    pos->next = item;
    if(item->next == NULL)
    {
      assert(pos == ll->last);
      ll->last = item;
    }
  }
}

static inline void lisp_dl_list_remove(lisp_dl_list_t * ll,
                                       lisp_dl_item_t * item)
{
  if(item->prev == NULL)
  {
    assert(ll->first == item);
    ll->first = item->next;
  }
  else
  {
    item->prev->next = item->next;
  }
  if(item->next == NULL)
  {
    assert(ll->last == item);
    ll->last = item->prev;
  }
  else
  {
    item->next->prev = item->prev;
  }
}

static inline void lisp_dl_list_remove_first(lisp_dl_list_t * ll)
{
  lisp_dl_list_remove(ll, ll->first);
}

#endif

