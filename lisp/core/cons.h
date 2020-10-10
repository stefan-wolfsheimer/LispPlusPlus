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
#ifndef __LIPS_CONS_H__
#define __LIPS_CONS_H__
#include <lisp/util/dl_list.h>
#include <stddef.h>

/* @todo move to different cell */
typedef struct lisp_cell_t
{
} lisp_cell_t;

struct lisp_vm_t;

typedef struct lisp_cons_t
{
  lisp_cell_t car;
  lisp_cell_t cdr;
} lisp_cons_t;

typedef struct lisp_cons_pages_t
{
  void ** pages;
  void * current_page;
  lisp_dl_list_t recycled;
  size_t page_size;
  size_t num_pages;
  size_t pos;
} lisp_cons_pages_t;

/**
 * Initialize cons allocator
 */
int lisp_init_cons_pages(lisp_cons_pages_t * pages);

int lisp_free_cons_pages(lisp_cons_pages_t * pages);

int lisp_cons_pages_set_size(lisp_cons_pages_t * pages, size_t page_size);


lisp_cons_t * lisp_cons_alloc(struct lisp_cons_pages_t * pages);

/** 
 * Frees an allocated cons
 */
void lisp_cons_free(lisp_cons_pages_t * pages, lisp_cons_t * cons);

#endif
