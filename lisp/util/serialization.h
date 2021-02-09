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
#ifndef __LISP_SERIALIZATION_H__
#define __LISP_SERIALIZATION_H__
#include <stddef.h>
#include <stdio.h>

struct serialization_field_t;

typedef struct serialization_field_t
{
  const char * name;
  const char * format;
  void * data;
  void *(*accessor)(void * obj);
  void (*initializer)(struct serialization_field_t * f,
                      void * obj,
                      void * context);
  int (*eq)(struct serialization_field_t * f,
            void * a,
            void * b);
  void (*print)(struct serialization_field_t * f,
                void * obj,
                FILE * fp,
                const char * format,
                void * context);
} serialization_field_t;

/**
 * Initialize object.
 */
void serialization_init(serialization_field_t * fields,
                        size_t n,
                        void * obj,
                        void * context);

/**
 * Return true if all fields of obj1 and obj2 are equal
 */
int serialization_eq(serialization_field_t * fields,
                     size_t n,
                     void * obj1,
                     void * obj2);

void serialization_print(FILE * fp,
                         serialization_field_t * fields,
                         size_t n,
                         void * obj1,
                         void * context);

void serialization_print2(FILE * fp,
                          serialization_field_t * fields,
                          size_t n,
                          void * obj1,
                          void * obj2,
                          int color,
                          void * context);


/**
 * Change the initialization of a given field
 */
void serialization_set_initializer(serialization_field_t * fields,
                                   size_t n,
                                   const char * name,
                                   void (*initializer)(serialization_field_t * f,
                                                       void * obj,
                                                       void * context));

/******************************************************************************
 * int function
 ******************************************************************************/
void serialization_init_int(serialization_field_t * f,
                            void * obj,
                            void * context);

void serialization_print_int(serialization_field_t * f,
                             void * obj,
                             FILE * fp,
                             const char * format,
                             void * context);
int serialization_eq_int(serialization_field_t * f,
                         void * a,
                         void * b);

#define SERIAL_INT_FIELD(__TYPE__,                                      \
                         __NAME__,                                      \
                         __LABEL__,                                     \
                         __FORMAT__,                                    \
                         __DATA__)                                      \
  { (__LABEL__ ? __LABEL__ : #__NAME__),                                \
      (__FORMAT__ ? __FORMAT__ : "%d"),                                 \
      (__DATA__),                                                       \
      __accessor__##__TYPE__##__NAME__,                                 \
      serialization_init_int,                                           \
      serialization_eq_int,                                             \
      serialization_print_int                                           \
  }

/******************************************************************************
 * size_t function
 ******************************************************************************/
void serialization_init_size(serialization_field_t * f,
                             void * obj,
                             void * context);

void serialization_print_size(serialization_field_t * f,
                              void * obj,
                              FILE * fp,
                              const char * format,
                              void * context);
int serialization_eq_size(serialization_field_t * f,
                          void * a,
                          void * b);

#define SERIAL_SIZE_FIELD(__TYPE__,                                     \
                         __NAME__,                                      \
                         __LABEL__,                                     \
                         __FORMAT__,                                    \
                         __DATA__)                                      \
  { (__LABEL__ ? __LABEL__ : #__NAME__),                                \
      (__FORMAT__ ? __FORMAT__ : "%zu"),                                \
      (__DATA__),                                                       \
      __accessor__##__TYPE__##__NAME__,                                 \
      serialization_init_size,                                          \
      serialization_eq_size,                                            \
      serialization_print_size                                          \
  }

/******************************************************************************
 * bool functions
 ******************************************************************************/
void serialization_init_bool(serialization_field_t * f,
                             void * obj,
                             void * context);

void serialization_print_bool(serialization_field_t * f,
                              void * obj,
                              FILE * fp,
                              const char * format,
                              void * context);
int serialization_eq_bool(serialization_field_t * f,
                          void * a,
                          void * b);

#define SERIAL_BOOL_FIELD(__TYPE__,                                     \
                          __NAME__,                                     \
                          __LABEL__,                                    \
                          __FORMAT__,                                   \
                          __DATA__)                                     \
  { ((__LABEL__) ? (__LABEL__) : (#__NAME__)),                          \
      ((__FORMAT__) ? (__FORMAT__) : "%zu"),                            \
      (__DATA__),                                                       \
      __accessor__##__TYPE__##__NAME__,                                 \
      serialization_init_bool,                                          \
      serialization_eq_bool,                                            \
      serialization_print_bool                                          \
  }

#define SERIAL_DEF_ACCESSOR(__TYPE__, __NAME__)                        \
  static void * __accessor__##__TYPE__##__NAME__(void * obj)           \
  { return &((__TYPE__*)(obj))->__NAME__; }

#define SERIAL_DEF_ARRAY_ACCESSOR(__TYPE__, __NAME__, __FIELD__, __I__) \
  static void * __accessor__##__TYPE__##__NAME__(void * obj)            \
  { return &((__TYPE__*)(obj))->__FIELD__[__I__]; }
    
#endif
