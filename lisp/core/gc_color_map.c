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
#include <lisp/util/xmalloc.h>
#include <lisp/core/error.h>
#include "gc_color_map.h"

/*****************************************************************************
 Helper functions
 ****************************************************************************/
static lisp_gc_collectible_list_t * _new_gc_collectible_list(lisp_gc_color_t c,
                                                             short int is_root,
                                                             struct lisp_vm_t * vm)
{
  lisp_gc_collectible_list_t * lst = MALLOC(sizeof(lisp_gc_collectible_list_t));
  if(lst)
  {
    lst->is_root = is_root;
    lisp_init_dl_list(&lst->objects);
    lst->color = c;
    lst->vm = vm;
  }
  return lst;
}

int lisp_init_collectible_lists(lisp_gc_collectible_list_t * lists[],
                                struct lisp_vm_t * vm)
{
  size_t i;
  for(i = 0; i < LISP_GC_NUM_CLASSES; i++)
  {
    lists[i] = _new_gc_collectible_list(i,
                                        LISP_GC_IS_ROOT_CLASS(i),
                                        vm);
    if(lists[i] == NULL)
    {
      return LISP_BAD_ALLOC;
    }
  }
  lisp_collectible_list_refresh(lists);
  return LISP_OK;
}

int lisp_free_collectible_lists(lisp_gc_collectible_list_t * lists[])
{
  size_t i;
  for(i = 0; i < LISP_GC_NUM_CLASSES; i++)
  {
    FREE(lists[i]);
  }
  return LISP_OK;
}

void lisp_collectible_list_refresh(lisp_gc_collectible_list_t * l[])
{
  /*
   * we don't know if another object still refers to unrooted objects
   * -> never transition from root to white
   */
  l[LISP_GC_WHITE_ROOT]->other_elements = l[LISP_GC_GREY];
  l[LISP_GC_GREY_ROOT ]->other_elements = l[LISP_GC_GREY];
  l[LISP_GC_BLACK_ROOT]->other_elements = l[LISP_GC_BLACK];
  l[LISP_GC_WHITE     ]->other_elements = l[LISP_GC_WHITE_ROOT];
  l[LISP_GC_GREY      ]->other_elements = l[LISP_GC_WHITE_ROOT];
  l[LISP_GC_BLACK     ]->other_elements = l[LISP_GC_WHITE_ROOT];

  l[LISP_GC_WHITE_ROOT]->grey_elements = l[LISP_GC_GREY_ROOT];
  l[LISP_GC_GREY_ROOT ]->grey_elements = NULL;
  l[LISP_GC_BLACK_ROOT]->grey_elements = NULL;
  l[LISP_GC_WHITE     ]->grey_elements = l[LISP_GC_GREY];
  l[LISP_GC_GREY      ]->grey_elements = NULL;
  l[LISP_GC_BLACK     ]->grey_elements = NULL;

  l[LISP_GC_WHITE_ROOT]->to_elements = l[LISP_GC_GREY_ROOT];
  l[LISP_GC_GREY_ROOT ]->to_elements = l[LISP_GC_BLACK_ROOT];
  l[LISP_GC_BLACK_ROOT]->to_elements = NULL;
  l[LISP_GC_WHITE     ]->to_elements = l[LISP_GC_GREY];
  l[LISP_GC_GREY      ]->to_elements = l[LISP_GC_BLACK];;
  l[LISP_GC_BLACK     ]->to_elements = NULL;
}
