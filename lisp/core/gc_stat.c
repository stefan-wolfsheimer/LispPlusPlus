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

#define _GC_STAT_FIELD_SIZE_T 0
#define _GC_STAT_FIELD_INT 1
#define _GC_STAT_FIELD_BOOL 2

#define _GC_STAT_COLOR_NORMAL   "\x1B[0m"
#define _GC_STAT_COLOR_RED      "\x1B[31m"
#define _GC_STAT_COLOR_GREEN    "\x1B[32m"
#define _GC_STAT_COLOR_YELLOW   "\x1B[33m"
#define _GC_STAT_COLOR_BLUE     "\x1B[34m"
#define _GC_STAT_COLOR_MAGENTA  "\x1B[35m"
#define _GC_STAT_COLOR_CYAN     "\x1B[36m"
#define _GC_STAT_COLOR_WHITE    "\x1B[37m"

#define _GC_STAT_GETTER(__NAME__)                         \
  static void * _get_##__NAME__(lisp_gc_stat_t * stat)    \
  { return &stat->__NAME__; }

#define _GC_STAT_SIZE_FIELD(__NAME__)                             \
  {#__NAME__, "% 5zu", _GC_STAT_FIELD_SIZE_T, _get_##__NAME__}    \

#define _GC_STAT_INT_FIELD(__NAME__)                            \
  {#__NAME__, "% 5d", _GC_STAT_FIELD_INT, _get_##__NAME__}      \

#define _GC_STAT_BOOL_FIELD(__NAME__)                           \
  {#__NAME__, "% 5d", _GC_STAT_FIELD_BOOL, _get_##__NAME__}     \

static void _gc_stat_print_field(FILE * fp,
                                 lisp_gc_stat_field_t * field,
                                 lisp_gc_stat_t * stat)
{
  switch(field->type)
  {
  case _GC_STAT_FIELD_SIZE_T:
    fprintf(fp, field->format, *((size_t*)field->getter(stat)));
    break;
  case _GC_STAT_FIELD_INT:
    fprintf(fp, field->format, *((int*)field->getter(stat)));
    break;
  case _GC_STAT_FIELD_BOOL:
    if(*((int*)field->getter(stat)))
    {
      fprintf(fp, " true");
    }
    else
    {
      fprintf(fp, "false");
    }
    break;
  }
}

static int _lisp_gc_stat_eq_field(lisp_gc_stat_field_t * field,
                                  lisp_gc_stat_t * stat1,
                                  lisp_gc_stat_t * stat2)
{
  switch(field->type)
  {
  case _GC_STAT_FIELD_SIZE_T:
    return (*((size_t*)field->getter(stat1)) ==
            *((size_t*)field->getter(stat2)));
  case _GC_STAT_FIELD_INT:
    return (*((int*)field->getter(stat1)) ==
            *((int*)field->getter(stat2)));
  case _GC_STAT_FIELD_BOOL:
    return (( *((int*)field->getter(stat1))? 1 : 0) ==
            ( *((int*)field->getter(stat2)) ? 1 : 0));
  }
  return 0;
}

static size_t _lisp_gc_stat_name_length()
{
  static size_t maxlen = 0;
  size_t i;
  size_t l;
  if(maxlen == 0)
  {
    for(i = 0; i < LISP_GC_STAT_NUM_FIELDS; i++)
    {
      l = strlen(lisp_gc_stat_field[i].name);
      if(l > maxlen)
      {
        maxlen = l;
      }
    }
    maxlen+= 2;
  }
  return maxlen;
}

_GC_STAT_GETTER(num_root);
_GC_STAT_GETTER(num_bulk);
_GC_STAT_GETTER(num_reachable);
_GC_STAT_GETTER(num_allocated);
_GC_STAT_GETTER(num_recycled);
_GC_STAT_GETTER(num_void);
_GC_STAT_GETTER(num_disposed);
_GC_STAT_GETTER(num_cycles);
_GC_STAT_GETTER(num_leaves);
_GC_STAT_GETTER(num_edges);
_GC_STAT_GETTER(num_cons_pages);
_GC_STAT_GETTER(num_white_root_conses);
_GC_STAT_GETTER(num_grey_root_conses);
_GC_STAT_GETTER(num_black_root_conses);
_GC_STAT_GETTER(num_white_conses);
_GC_STAT_GETTER(num_grey_conses);
_GC_STAT_GETTER(num_black_conses);
_GC_STAT_GETTER(num_white_root_objects);
_GC_STAT_GETTER(num_grey_root_objects);
_GC_STAT_GETTER(num_black_root_objects);
_GC_STAT_GETTER(num_white_objects);
_GC_STAT_GETTER(num_grey_objects);
_GC_STAT_GETTER(num_black_objects);
_GC_STAT_GETTER(error_black_has_white_child);
_GC_STAT_GETTER(error_list_white_root_conses);
_GC_STAT_GETTER(error_list_grey_root_conses);
_GC_STAT_GETTER(error_list_black_root_conses);
_GC_STAT_GETTER(error_list_white_conses);
_GC_STAT_GETTER(error_list_grey_conses);
_GC_STAT_GETTER(error_list_black_conses);
_GC_STAT_GETTER(error_list_white_root_object);
_GC_STAT_GETTER(error_list_grey_root_object);
_GC_STAT_GETTER(error_list_black_root_object);
_GC_STAT_GETTER(error_list_white_object);
_GC_STAT_GETTER(error_list_grey_object);
_GC_STAT_GETTER(error_list_black_object);


lisp_gc_stat_field_t lisp_gc_stat_field[LISP_GC_STAT_NUM_FIELDS] =
  {
    _GC_STAT_SIZE_FIELD(num_root),
    _GC_STAT_SIZE_FIELD(num_bulk),
    _GC_STAT_SIZE_FIELD(num_reachable),
    _GC_STAT_SIZE_FIELD(num_allocated),
    _GC_STAT_SIZE_FIELD(num_recycled),
    _GC_STAT_SIZE_FIELD(num_void),
    _GC_STAT_SIZE_FIELD(num_disposed),
    _GC_STAT_SIZE_FIELD(num_cycles),
    _GC_STAT_SIZE_FIELD(num_leaves),
    _GC_STAT_SIZE_FIELD(num_edges),
    _GC_STAT_SIZE_FIELD(num_cons_pages),
    _GC_STAT_SIZE_FIELD(num_white_root_conses),
    _GC_STAT_SIZE_FIELD(num_grey_root_conses),
    _GC_STAT_SIZE_FIELD(num_black_root_conses),
    _GC_STAT_SIZE_FIELD(num_white_conses),
    _GC_STAT_SIZE_FIELD(num_grey_conses),
    _GC_STAT_SIZE_FIELD(num_black_conses),
    _GC_STAT_SIZE_FIELD(num_white_root_objects),
    _GC_STAT_SIZE_FIELD(num_grey_root_objects),
    _GC_STAT_SIZE_FIELD(num_black_root_objects),
    _GC_STAT_SIZE_FIELD(num_white_objects),
    _GC_STAT_SIZE_FIELD(num_grey_objects),
    _GC_STAT_SIZE_FIELD(num_black_objects),
    _GC_STAT_BOOL_FIELD(error_black_has_white_child),
    _GC_STAT_BOOL_FIELD(error_list_white_root_conses),
    _GC_STAT_BOOL_FIELD(error_list_grey_root_conses),
    _GC_STAT_BOOL_FIELD(error_list_black_root_conses),
    _GC_STAT_BOOL_FIELD(error_list_white_conses),
    _GC_STAT_BOOL_FIELD(error_list_grey_conses),
    _GC_STAT_BOOL_FIELD(error_list_black_conses),
    _GC_STAT_BOOL_FIELD(error_list_white_root_object),
    _GC_STAT_BOOL_FIELD(error_list_grey_root_object),
    _GC_STAT_BOOL_FIELD(error_list_black_root_object),
    _GC_STAT_BOOL_FIELD(error_list_white_object),
    _GC_STAT_BOOL_FIELD(error_list_grey_object),
    _GC_STAT_BOOL_FIELD(error_list_black_object)
  };

void lisp_init_gc_stat(lisp_gc_stat_t * stat)
{
  size_t i;
  for(i = 0; i < LISP_GC_STAT_NUM_FIELDS; i++)
  {
    switch(lisp_gc_stat_field[i].type)
    {
    case _GC_STAT_FIELD_SIZE_T:
      *((size_t*)lisp_gc_stat_field[i].getter(stat)) = 0u;
      break;
    case _GC_STAT_FIELD_INT:
      *((int*)lisp_gc_stat_field[i].getter(stat)) = 0;
      break;
    case _GC_STAT_FIELD_BOOL:
      *((int*)lisp_gc_stat_field[i].getter(stat)) = 0;
      break;
    }
  }
}

int lisp_gc_stat_eq(lisp_gc_stat_t * stat1,
                    lisp_gc_stat_t * stat2)
{
  int ret = 1;
  size_t i;
  for(i = 0; i < LISP_GC_STAT_NUM_FIELDS; i++)
  {
    ret&= _lisp_gc_stat_eq_field(&lisp_gc_stat_field[i],
                                 stat1,
                                 stat2);
  }
  return ret;
}

void lisp_gc_stat_print(FILE * fp, lisp_gc_stat_t * stat)
{
  size_t maxlen;
  size_t i, j, l;
  maxlen = _lisp_gc_stat_name_length();
  for(i = 0; i < LISP_GC_STAT_NUM_FIELDS; i++)
  {
    l = strlen(lisp_gc_stat_field[i].name);
    fprintf(fp, "%s:", lisp_gc_stat_field[i].name);
    for(j = l; j < maxlen; j++)
    {
      fputc(' ', fp);
    }
    _gc_stat_print_field(fp, &lisp_gc_stat_field[i], stat);
    fputc('\n', fp);
  }
}

void lisp_gc_stat_print2(FILE * fp,
                         lisp_gc_stat_t * stat1,
                         lisp_gc_stat_t * stat2)
{
  size_t maxlen;
  size_t i, j, l;
  int ok;
  maxlen = _lisp_gc_stat_name_length();
  for(i = 0; i < LISP_GC_STAT_NUM_FIELDS; i++)
  {
    l = strlen(lisp_gc_stat_field[i].name);
    ok = _lisp_gc_stat_eq_field(&lisp_gc_stat_field[i], stat1, stat2);
#if LISP_GC_STAT_PRINT_COLOR
    if(!ok)
    {
      fprintf(fp, _GC_STAT_COLOR_RED);
    }
#endif
    fprintf(fp, "%s:", lisp_gc_stat_field[i].name);
    for(j = l; j < maxlen; j++)
    {
      fputc(' ', fp);
    }
    _gc_stat_print_field(fp, &lisp_gc_stat_field[i], stat1);
    fputc(' ', fp);
    _gc_stat_print_field(fp, &lisp_gc_stat_field[i], stat2);
#if LISP_GC_STAT_PRINT_COLOR
    if(!ok)
    {
      fprintf(fp, _GC_STAT_COLOR_NORMAL);
    }
#endif
    fputc('\n', fp);
  }
}

#undef _GC_STAT_FIELD_SIZE_T
#undef _GC_STAT_FIELD_INT
#undef _GC_STAT_FIELD_BOOL

#undef _GC_STAT_COLOR_NORMAL
#undef _GC_STAT_COLOR_RED
#undef _GC_STAT_COLOR_GREEN
#undef _GC_STAT_COLOR_YELLOW
#undef _GC_STAT_COLOR_BLUE
#undef _GC_STAT_COLOR_MAGENTA
#undef _GC_STAT_COLOR_CYAN
#undef _GC_STAT_COLOR_WHITE

#undef _GC_STAT_GETTER
#undef _GC_STAT_SIZE_FIELD
#undef _GC_STAT_INT_FIELD
#undef _GC_STAT_BOOL_FIELD
