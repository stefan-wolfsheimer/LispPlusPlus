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
#include "gc_stat.h"
#include <string.h>

SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_root);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_bulk);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_reachable);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_allocated);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_recycled);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_void);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_disposed);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_cycles);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_leaves);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_edges);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t, num_cons_pages);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_white_root_conses,
                          num_conses,
                          LISP_GC_WHITE_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_grey_root_conses,
                          num_conses,
                          LISP_GC_GREY_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_black_root_conses,
                          num_conses,
                          LISP_GC_BLACK_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_white_conses,
                          num_conses,
                          LISP_GC_WHITE);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_grey_conses,
                          num_conses,
                          LISP_GC_GREY);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_black_conses,
                          num_conses,
                          LISP_GC_BLACK);

SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_white_root_objects,
                          num_objects,
                          LISP_GC_WHITE_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_grey_root_objects,
                          num_objects,
                          LISP_GC_GREY_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_black_root_objects,
                          num_objects,
                          LISP_GC_BLACK_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_white_objects,
                          num_objects,
                          LISP_GC_WHITE);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_grey_objects,
                          num_objects,
                          LISP_GC_GREY);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          num_black_objects,
                          num_objects,
                          LISP_GC_BLACK);
SERIAL_DEF_ACCESSOR(lisp_gc_stat_t,
                    error_black_has_white_child);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_white_root_conses,
                          error_cons_lists,
                          LISP_GC_WHITE_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_grey_root_conses,
                          error_cons_lists,
                          LISP_GC_GREY_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_black_root_conses,
                          error_cons_lists,
                          LISP_GC_BLACK_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_white_conses,
                          error_cons_lists,
                          LISP_GC_WHITE);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_grey_conses,
                          error_cons_lists,
                          LISP_GC_GREY);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_black_conses,
                          error_cons_lists,
                          LISP_GC_BLACK);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_white_root_objects,
                          error_object_lists,
                          LISP_GC_WHITE_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_grey_root_objects,
                          error_object_lists,
                          LISP_GC_GREY_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_black_root_objects,
                          error_object_lists,
                          LISP_GC_BLACK_ROOT);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_white_objects,
                          error_object_lists,
                          LISP_GC_WHITE);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_grey_objects,
                          error_object_lists,
                          LISP_GC_GREY);
SERIAL_DEF_ARRAY_ACCESSOR(lisp_gc_stat_t,
                          error_list_black_objects,
                          error_object_lists,
                          LISP_GC_BLACK);

serialization_field_t lisp_gc_stat_fields[LISP_GC_STAT_NUM_FIELDS] =
  {
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_root,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_bulk,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_reachable,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_allocated,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_recycled,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_void,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_disposed,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_cycles,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_leaves,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_edges,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_cons_pages,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_white_root_conses,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_grey_root_conses,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_black_root_conses,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_white_conses,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_grey_conses,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_black_conses,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_white_root_objects,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_grey_root_objects,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_black_root_objects,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_white_objects,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_grey_objects,
                      NULL, NULL, NULL),
    SERIAL_SIZE_FIELD(lisp_gc_stat_t,
                      num_black_objects,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_black_has_white_child,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_white_root_conses,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_grey_root_conses,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_black_root_conses,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_white_conses,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_grey_conses,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_black_conses,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_white_root_objects,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_grey_root_objects,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_black_root_objects,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_white_objects,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_grey_objects,
                      NULL, NULL, NULL),
    SERIAL_BOOL_FIELD(lisp_gc_stat_t,
                      error_list_black_objects,
                      NULL, NULL, NULL)

  };

void lisp_init_gc_stat(lisp_gc_stat_t * stat)
{
  serialization_init(lisp_gc_stat_fields,
                     LISP_GC_STAT_NUM_FIELDS,
                     stat,
                     NULL);
}

int lisp_gc_stat_eq(lisp_gc_stat_t * stat1,
                    lisp_gc_stat_t * stat2)
{
  return serialization_eq(lisp_gc_stat_fields,
                          LISP_GC_STAT_NUM_FIELDS,
                          stat1,
                          stat2);
}

void lisp_gc_stat_print(FILE * fp, lisp_gc_stat_t * stat)
{
  serialization_print(fp,
                      lisp_gc_stat_fields,
                      LISP_GC_STAT_NUM_FIELDS,
                      stat,
                      NULL);
}

void lisp_gc_stat_print2(FILE * fp,
                         lisp_gc_stat_t * stat1,
                         lisp_gc_stat_t * stat2)
{
  int color;
#if LISP_GC_STAT_PRINT_COLOR
  color = 1;
#else
  color = 0;
#endif
  serialization_print2(fp,
                       lisp_gc_stat_fields,
                       LISP_GC_STAT_NUM_FIELDS,
                       stat1,
                       stat2,
                       color,
                       NULL);
}
