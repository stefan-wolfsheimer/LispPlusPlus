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

argument_t * arg_parser_add_field(arg_parser_t * parser,
                                  void * ref,
                                  char short_name,
                                  const char * long_name,
                                  int required,
                                  int (*parse)(struct arg_parser_t * parser,
                                               struct argument_t * arg,
                                               const char ** values,
                                               size_t n_values),
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
  arg->short_name = short_name;
  arg->long_name = string_copy(long_name);
  arg->help = string_copy(help);
  arg->multiplier = '1';
  arg->required = required;
  arg->data = ref;
  arg->parse = parse;
  return arg;
}

static int _parse_flag(arg_parser_t * parser,
                      argument_t * arg,
                      const char ** values,
                      size_t n_values)
{
  if(n_values != 0)
  {
    return ARG_PARSER_SETUP_ERROR;
  }
  *((int*)(arg->data)) = 1;
  return ARG_PARSER_OK;
}

argument_t * arg_parser_add_flag(arg_parser_t * parser,
                                 int * ref,
                                 char short_name,
                                 const char * long_name,
                                 const char * help)
{
  argument_t * arg;
  *ref = 0;
  arg = arg_parser_add_field(parser,
                             ref,
                             short_name,
                             long_name,
                             0,
                             _parse_flag,
                             help);
  arg->multiplier = '0';
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
                                char short_name,
                                const char * long_name,
                                int required,
                                const char * help)
{
  return arg_parser_add_field(parser,
                              ref,
                              short_name,
                              long_name,
                              required,
                              _parse_int,
                              help);
}

/**
 * argument has the form "-%c"
 */
static char _is_short_arg(const char * argvi)
{
  if(*argvi != '-')
  {
    return '\0';
  }
  ++argvi;
  if(*argvi == '\0' || *argvi == '-')
  {
    return '\0';
  }
  if(argvi[1] == '\0')
  {
    return *argvi;
  }
  return '\0';
}

static argument_t * _find_argument_by_short_name(arg_parser_t * parser,
                                                 char short_arg)
{
  argument_t * arg;
  size_t j;
  for(j = 0; j < parser->n_arguments; j++)
  {
    arg = parser->arguments[j];
    if(arg->short_name != '\0')
    {
      if(short_arg == arg->short_name)
      {
        return arg;
      }
    }
  }
  return NULL;
}

static argument_t * _find_argument(arg_parser_t * parser,
                                   const char * argvi)
{
  argument_t * arg;
  char short_arg;
  size_t j;
  short_arg = _is_short_arg(argvi);
  if(short_arg)
  {
    return _find_argument_by_short_name(parser, short_arg);
  }
  for(j = 0; j < parser->n_arguments; j++)
  {
    arg = parser->arguments[j];
    if(arg->long_name && !strcmp(argvi, arg->long_name))
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

static int _is_multiple_short_arg(const char * argvi)
{
  if(*argvi == '-')
  {
    ++argvi;
    if(*argvi == '\0' || *argvi == '-')
    {
      return 0;
    }
    ++argvi;
    if(*argvi == '\0')
    {
      return 0;
    }
    return 1;
  }
  return 0;
}

static int _parse_arg(arg_parser_t * parser,
                      argument_t * arg,
                      const char ** argv,
                      int * refi,
                      int argc)
{
  size_t l;
  int parseres;
  switch(arg->multiplier)
  {
  case '0':
    ++(*refi);
    parseres = arg->parse(parser, arg, argv + *refi, 0);
    return parseres;
  case '1':
    if((*refi + 1) < argc)
    {
      ++(*refi);
      return arg->parse(parser, arg, argv + *refi, 1);
    }
    else
    {
      /* @todo error message */
      return ARG_PARSER_MISSING_VALUE;
    }
    break;
  case '+':
    ++(*refi);
    l = _vararg_length(argc, argv, *refi);
    if(l == 0)
    {
      /* @todo error message */
      return ARG_PARSER_MISSING_VALUE;
    }
    parseres = arg->parse(parser, arg, argv + *refi, l);
    *refi += l;
    return parseres;
    break;
  case '*':
    ++(*refi);
    l = _vararg_length(argc, argv, *refi);
    parseres = arg->parse(parser, arg, argv + *refi, l);
    (*refi) += l;
    return parseres;
    break;
  default:
    return ARG_PARSER_SETUP_ERROR;
    break;
  }
  return 0;
}

int arg_parser_parse(arg_parser_t * parser,
                     int argc,
                     const char ** argv)
{
  int i;
  int res;
  argument_t * arg;
  const char * argvi;
  const char * ptr;
  for(i = 0; i < argc; i++)
  {
    argvi = argv[i];
    if(_is_multiple_short_arg(argvi))
    {
      ptr = argvi;
      ++ptr;
      while(*ptr)
      {
        arg = _find_argument_by_short_name(parser, *ptr);
        if(arg == NULL)
        {
          /* @todo error message */
          return ARG_PARSER_INVALID_ARGUMENT;
        }
        if(arg->multiplier == '0')
        {
          res = arg->parse(parser, arg, argv + i, 0);
        }
        else
        {
          res = ARG_PARSER_INVALID_ARGUMENT;
        }
        if(res != ARG_PARSER_OK)
        {
          return res;
        }
        ++ptr;
      }
    }
    else
    {
      arg = _find_argument(parser, argvi);
      if(arg == NULL)
      {
        /* @todo error message */
        return ARG_PARSER_INVALID_ARGUMENT;
      }
      res = _parse_arg(parser, arg, argv, &i, argc);
      if(res != ARG_PARSER_OK)
      {
        return res;
      }
    }
  } /* foreach argv */
  return ARG_PARSER_OK;
}


