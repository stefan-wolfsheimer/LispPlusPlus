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
#ifndef __ARGPARSE_H__
#define __ARGPARSE_H__
#include <stddef.h>
#include <stdio.h>

#define ARG_PARSER_OK 0


/**
 * invalid usage of argument parser
 */
#define ARG_PARSER_SETUP_ERROR 1

#define ARG_PARSER_MISSING_VALUE 2

#define ARG_PARSER_INVALID_ARGUMENT 3

#define ARG_PARSER_OVERFLOW 4


struct arg_parser_t;
struct argument_t;

typedef struct argument_t
{
  const char * short_name;
  const char * long_name;
  const char * help;
  char multiplier; /* '0': argument is a flag, no values
                    * '1': argument takes exactly one value
                    * '+': argument takes at least one value
                    */
  int required;

  size_t min_n_values;
  size_t max_n_values;
  int status; /* status of the argument:
                 ARG_PARSER_OK (default on initialization)
                 ARG_PARSER_MISSING_VALUE (argument has been passed
                                           without value)
              */
  void * data;
  int (*parse)(struct arg_parser_t * parser,
               struct argument_t * arg,
               const char ** values,
               size_t n_values);
} argument_t;

typedef struct arg_parser_t
{
  argument_t ** arguments;
  size_t n_arguments;
  char * error_msg;
} arg_parser_t;

int init_arg_parser(arg_parser_t * parser);

int free_arg_parser(arg_parser_t * parser);

argument_t * arg_parser_add_int(arg_parser_t * parser,
                                int * ref,
                                const char * short_name,
                                const char * long_name,
                                int required,
                                const char * help);

int arg_parser_parse(arg_parser_t * parser,
                     int argc,
                     const char ** argv);
#endif
