#include "serialization.h"
#include <string.h>
#include <stdio.h>

#define _SERIAL_COLOR_NORMAL   "\x1B[0m"
#define _SERIAL_COLOR_RED      "\x1B[31m"
#define _SERIAL_COLOR_GREEN    "\x1B[32m"
#define _SERIAL_COLOR_YELLOW   "\x1B[33m"
#define _SERIAL_COLOR_BLUE     "\x1B[34m"
#define _SERIAL_COLOR_MAGENTA  "\x1B[35m"
#define _SERIAL_COLOR_CYAN     "\x1B[36m"
#define _SERIAL_COLOR_WHITE    "\x1B[37m"


void serialization_init(serialization_field_t * fields,
                        size_t n,
                        void * obj,
                        void * context)
{
  size_t i;
  for(i = 0; i < n; i++)
  {
    fields[i].initializer(&fields[i],
                          obj,
                          context);
  }
}

int serialization_eq(serialization_field_t * fields,
                     size_t n,
                     void * obj1,
                     void * obj2)
{
  size_t i;
  int ret = 1;
  for(i = 0; i < n; i++)
  {
    ret&= fields[i].eq(&fields[i],
                       obj1,
                       obj2);
  }
  return ret;
}

static size_t _serialization_name_length(serialization_field_t * fields,
                                         size_t n)
{
  size_t i, l, maxlen;
  maxlen = 0;
  for(i = 0; i < n; i++)
  {
    l = strlen(fields[i].name);
    if(l > maxlen)
    {
      maxlen = l;
    }
  }
  maxlen+= 2;
  return maxlen;
}

void serialization_print(FILE * fp,
                         serialization_field_t * fields,
                         size_t n,
                         void * obj,
                         void * context)
{
  size_t maxlen;
  size_t i, j, l;
  maxlen = _serialization_name_length(fields, n);
  for(i = 0; i < n; i++)
  {
    l = strlen(fields[i].name);
    fprintf(fp, "%s:", fields[i].name);
    for(j = l; j < maxlen; j++)
    {
      fputc(' ', fp);
    }
    fields[i].print(&fields[i], obj, fp, fields[i].format, context);
    fputc('\n', fp);
  }
}

void serialization_print2(FILE * fp,
                          serialization_field_t * fields,
                          size_t n,
                          void * obj1,
                          void * obj2,
                          int color,
                          void * context)
{
  size_t maxlen;
  size_t i, j, l;
  int eq;
  maxlen = _serialization_name_length(fields, n);
  for(i = 0; i < n; i++)
  {
    l = strlen(fields[i].name);
    eq = fields[i].eq(&fields[i], obj1, obj2);
    if(!eq && color)
    {
      fprintf(fp, _SERIAL_COLOR_RED);
    }
    fprintf(fp, "%s:", fields[i].name);
    for(j = l; j < maxlen; j++)
    {
      fputc(' ', fp);
    }
    fields[i].print(&fields[i], obj1, fp, fields[i].format, context);
    fputc(' ', fp);
    fields[i].print(&fields[i], obj2, fp, fields[i].format, context);
    if(!eq && color)
    {
      fprintf(fp, _SERIAL_COLOR_NORMAL);
    }
    fputc('\n', fp);
  }
}

void serialization_set_initializer(serialization_field_t * fields,
                                   size_t n,
                                   const char * name,
                                   void (*initializer)(serialization_field_t * f,
                                                       void * obj,
                                                       void * context))
{
  size_t i;
  for(i = 0; i < n; i++)
  {
    if(!strcmp(name, fields[i].name))
    {
      fields[i].initializer = initializer;
    }
  }
}

/******************************************************************************
 * int function
 ******************************************************************************/
void serialization_init_int(serialization_field_t * f,
                            void * obj,
                            void * context)
{
  int * ptr = f->accessor(obj);
  *ptr = 0;
}

void serialization_print_int(serialization_field_t * f,
                             void * obj,
                             FILE * fp,
                             const char * format,
                             void * context)
{
  int * ptr = f->accessor(obj);
  fprintf(fp, f->format, *ptr);
}

int serialization_eq_int(serialization_field_t * f,
                          void * a,
                          void * b)
{
  int * ia = f->accessor(a);
  int * ib = f->accessor(b);
  return *ia == *ib;
}

/******************************************************************************
 * size_t functions
 ******************************************************************************/
void serialization_init_size(serialization_field_t * f,
                             void * obj,
                             void * context)
{
  size_t * ptr = f->accessor(obj);
  *ptr = 0;

}

void serialization_print_size(serialization_field_t * f,
                              void * obj,
                              FILE * fp,
                              const char * format,
                              void * context)
{
  size_t * ptr = f->accessor(obj);
  fprintf(fp, f->format, *ptr);
}

int serialization_eq_size(serialization_field_t * f,
                           void * a,
                           void * b)
{
  size_t * ia = f->accessor(a);
  size_t * ib = f->accessor(b);
  return *ia == *ib;
}

/******************************************************************************
 * bool functions
 ******************************************************************************/
void serialization_init_bool(serialization_field_t * f,
                             void * obj,
                             void * context)
{
  int * ptr = f->accessor(obj);
  *ptr = 0;
}

void serialization_print_bool(serialization_field_t * f,
                              void * obj,
                              FILE * fp,
                              const char * format,
                              void * context)
{
  /* @todo handle different formats */
  int * ptr = f->accessor(obj);
  if(*ptr)
  {
    fprintf(fp, "true");
  }
  else
  {
    fprintf(fp, "false");
  }
}

int serialization_eq_bool(serialization_field_t * f,
                          void * a,
                          void * b)
{
  int * ia = f->accessor(a);
  int * ib = f->accessor(b);
  return (*ia ? 1 : 0) == (*ib ? 1 : 0);
}

#undef _SERIAL_COLOR_NORMAL
#undef _SERIAL_COLOR_RED
#undef _SERIAL_COLOR_GREEN
#undef _SERIAL_COLOR_YELLOW
#undef _SERIAL_COLOR_BLUE
#undef _SERIAL_COLOR_MAGENTA
#undef _SERIAL_COLOR_CYAN
#undef _SERIAL_COLOR_WHITE
