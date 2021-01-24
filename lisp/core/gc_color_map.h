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
#ifndef __LISP_GC_COLOR_MAP_H__
#define __LISP_GC_COLOR_MAP_H__
#include <lisp/util/dl_list.h>
#include "gc_color.h"

/**
 * List of objects in garbage collector
 * generation.
 */
typedef struct lisp_gc_collectible_list_t
{
  lisp_dl_list_t objects;
  lisp_gc_color_t color;
  short int is_root;
  struct lisp_gc_collectible_list_t * other_elements;
  struct lisp_gc_collectible_list_t * grey_elements;
  struct lisp_gc_collectible_list_t * to_elements;
} lisp_gc_collectible_list_t;

typedef struct lisp_gc_color_map_t
{
  lisp_gc_collectible_list_t * white;
  lisp_gc_collectible_list_t * grey;
  lisp_gc_collectible_list_t * black;

  lisp_gc_collectible_list_t * white_root;
  lisp_gc_collectible_list_t * grey_root;
  lisp_gc_collectible_list_t * black_root;

} lisp_gc_color_map_t;

int lisp_init_color_map(lisp_gc_color_map_t * map);
int lisp_free_color_map(lisp_gc_color_map_t * map);

#endif
