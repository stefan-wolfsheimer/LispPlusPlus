#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "argparse.h"
#include <lisp/util/xmalloc.h>

int init_arg_parser(arg_parser_t * parser)
{
  parser->n_arguments = 0;
  parser->arguments = NULL;
  parser->error_msg = NULL;
  return 0;
}

int free_arg_parser(arg_parser_t * parser)
{
  size_t i;
  argument_t * arg;
  if(parser->arguments)
  {
    for(i = 0; i < parser->n_arguments; i++)
    {
      arg = parser->arguments[i];
      if(arg->short_name) 
      {
        FREE((void*)arg->short_name);
      }
      if(arg->long_name)
      {
        FREE((void*)arg->long_name);
      }
      if(arg->help)
      {
        FREE((void*)arg->help);
      }
      FREE(arg);
    }
    FREE(parser->arguments);
  }
  if(parser->error_msg)
  {
    FREE(parser->error_msg);
    parser->error_msg = NULL;
  }
  parser->n_arguments = 0;
  parser->arguments = NULL;
  return 0;
}

static char * string_copy(const char * str)
{
  char * ret;
  if(str)
  {
    ret = MALLOC(strlen(str) + 1);
    if(ret)
    {
      strcpy(ret, str);
    }
    return ret;
  }
  else
  {
    return NULL;
  }
}

static argument_t * arg_parser_add_field(arg_parser_t * parser,
                                         void * ref,
                                         const char * short_name,
                                         const char * long_name,
                                         int required,
                                         const char * help)
{
  size_t n_arguments;
  argument_t ** arguments;
  argument_t * arg;
  arg = MALLOC(sizeof(argument_t));
  if(arg == NULL)
  {
    return NULL;
  }
  n_arguments = parser->n_arguments + 1;
  arguments = REALLOC(parser->arguments,
                      sizeof(argument_t*) * n_arguments);
  if(arguments == NULL)
  {
    FREE(arg);
    return NULL;
  }
  parser->n_arguments = n_arguments;
  parser->arguments = arguments;
  parser->arguments[n_arguments - 1] = arg;
  arg->short_name = string_copy(short_name);
  arg->long_name = string_copy(long_name);
  arg->help = string_copy(help);
  arg->multiplier = '1';
  arg->required = required;
  arg->data = ref;
  arg->parse = NULL;
  return arg;
}

static int _parse_int(arg_parser_t * parser,
                      argument_t * arg,
                      const char ** values,
                      size_t n_values)
{
  long lv;
  char * end_ptr;
  if(n_values != 1)
  {
    return ARG_PARSER_SETUP_ERROR;
  }
  lv = strtol(values[0], &end_ptr, 10);
  if(*end_ptr != '\0')
  {
    /* @todo error message */
    return ARG_PARSER_INVALID_ARGUMENT;
  }
  if(lv < INT_MIN || lv > INT_MAX)
  {
    /* @todo error message */
    return ARG_PARSER_OVERFLOW;
  }
  *((int*)(arg->data)) = lv;
  return ARG_PARSER_OK;
}


argument_t * arg_parser_add_int(arg_parser_t * parser,
                                int * ref,
                                const char * short_name,
                                const char * long_name,
                                int required,
                                const char * help)
{
  argument_t * arg;
  arg = arg_parser_add_field(parser,
                             ref,
                             short_name,
                             long_name,
                             required,
                             help);
  arg->parse = _parse_int;
  return arg;
}

static argument_t * _find_argument(arg_parser_t * parser,
                                   const char * argvi)
{
  argument_t * arg;
  size_t j;
  for(j = 0; j < parser->n_arguments; j++)
  {
    arg = parser->arguments[j];
    if((arg->short_name && !strcmp(argvi, arg->short_name)) ||
       (arg->long_name && !strcmp(argvi, arg->long_name)))
    {
      return arg;
    }
  }
  return NULL;
}

static size_t _vararg_length(int argc, const char ** argv, int i)
{
  size_t l = 0;
  while(i < argc)
  {
    if(argv[i][0] == '-')
    {
      return l;
    }
    l++;
    i++;
  }
  return l;
}


int arg_parser_parse(arg_parser_t * parser,
                     int argc,
                     const char ** argv)
{
  int i;
  argument_t * arg;
  int parseres;
  size_t l;
  for(i = 0; i < argc; i++)
  {
    arg = _find_argument(parser, argv[i]);
    if(arg == NULL)
    {
      /* @todo error message */
      return ARG_PARSER_INVALID_ARGUMENT;
    }
    switch(arg->multiplier)
    {
    case '0':
      ++i;
      parseres = arg->parse(parser, arg, argv + i, 0);
      if(parseres != ARG_PARSER_OK)
      {
        return parseres;
      }
      break;
    case '1':
      if((i + 1) < argc)
      {
        ++i;
        parseres = arg->parse(parser, arg, argv + i, 1);  
        if(parseres != ARG_PARSER_OK)
        {
          return parseres;
        }
      }
      else
      {
        /* @todo error message */
        return ARG_PARSER_MISSING_VALUE;
      }
      break;
    case '+':
      ++i;
      l = _vararg_length(argc, argv, i);
      if(l == 0)
      {
        /* @todo error message */
        return ARG_PARSER_MISSING_VALUE;
      }
      parseres = arg->parse(parser, arg, argv + i, l);  
      i+= l;
      break;
    case '*':
      ++i;
      l = _vararg_length(argc, argv, i);
      parseres = arg->parse(parser, arg, argv + i, l);  
      i+= l;
      break;
    default:
      return ARG_PARSER_SETUP_ERROR;
      break;
    }
  } /* foreach argv */
  return ARG_PARSER_OK;
}


