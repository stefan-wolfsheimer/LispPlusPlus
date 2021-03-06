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
#include "tid.h"
#include "error.h"
#include "cons.h"
#include "array.h"
#include <stddef.h>

#define EMPTY_TYPE { NULL, \
                     NULL, \
                     NULL, \
                     NULL, \
                     NULL}

#define REGISTER_TYPE( __ID__, __FUNC__ )               \
  {                                                     \
    ret = __FUNC__(&lisp_static_types[(__ID__)]);       \
    if(ret != LISP_OK)                                  \
    {                                                   \
      return ret;                                       \
    }                                                   \
  }

int lisp_load_static_types()
{
  static int is_loaded = 0;
  int ret = LISP_OK;
  if(!is_loaded)
  {
    REGISTER_TYPE(LISP_TID_CONS, lisp_init_cons_type);
    REGISTER_TYPE(LISP_TID_ARRAY, lisp_init_array_type);
    is_loaded = 1;
  }
  return ret;
}

lisp_type_t lisp_static_types[LISP_NUM_TYPES] =
  {
    /* 0x00 */ EMPTY_TYPE,
    /* 0x01 */ EMPTY_TYPE,
    /* 0x02 */ EMPTY_TYPE,
    /* 0x03 */ EMPTY_TYPE,
    /* 0x04 */ EMPTY_TYPE, 
    /* 0x05 */ EMPTY_TYPE,
    /* 0x06 */ EMPTY_TYPE,
    /* 0x07 */ EMPTY_TYPE,
    /* 0x08 */ EMPTY_TYPE, 
    /* 0x09 */ EMPTY_TYPE,
    /* 0x0a */ EMPTY_TYPE,
    /* 0x0b */ EMPTY_TYPE,
    /* 0x0c */ EMPTY_TYPE, 
    /* 0x0d */ EMPTY_TYPE,
    /* 0x0e */ EMPTY_TYPE,
    /* 0x0f */ EMPTY_TYPE,

    /* 0x10 */ EMPTY_TYPE, 
    /* 0x11 */ EMPTY_TYPE,
    /* 0x12 */ EMPTY_TYPE,
    /* 0x13 */ EMPTY_TYPE,
    /* 0x14 */ EMPTY_TYPE, 
    /* 0x15 */ EMPTY_TYPE,
    /* 0x16 */ EMPTY_TYPE,
    /* 0x17 */ EMPTY_TYPE,
    /* 0x18 */ EMPTY_TYPE, 
    /* 0x19 */ EMPTY_TYPE,
    /* 0x1a */ EMPTY_TYPE,
    /* 0x1b */ EMPTY_TYPE,
    /* 0x1c */ EMPTY_TYPE, 
    /* 0x1d */ EMPTY_TYPE,
    /* 0x1e */ EMPTY_TYPE,
    /* 0x1f */ EMPTY_TYPE,

    /* 0x20 */ EMPTY_TYPE, 
    /* 0x21 */ EMPTY_TYPE,
    /* 0x22 */ EMPTY_TYPE,
    /* 0x23 */ EMPTY_TYPE,
    /* 0x24 */ EMPTY_TYPE, 
    /* 0x25 */ EMPTY_TYPE,
    /* 0x26 */ EMPTY_TYPE,
    /* 0x27 */ EMPTY_TYPE,
    /* 0x28 */ EMPTY_TYPE, 
    /* 0x29 */ EMPTY_TYPE,
    /* 0x2a */ EMPTY_TYPE,
    /* 0x2b */ EMPTY_TYPE,
    /* 0x2c */ EMPTY_TYPE, 
    /* 0x2d */ EMPTY_TYPE,
    /* 0x2e */ EMPTY_TYPE,
    /* 0x2f */ EMPTY_TYPE,

    /* 0x30 */ EMPTY_TYPE, 
    /* 0x31 */ EMPTY_TYPE,
    /* 0x32 */ EMPTY_TYPE,
    /* 0x33 */ EMPTY_TYPE,
    /* 0x34 */ EMPTY_TYPE, 
    /* 0x35 */ EMPTY_TYPE,
    /* 0x36 */ EMPTY_TYPE,
    /* 0x37 */ EMPTY_TYPE,
    /* 0x38 */ EMPTY_TYPE, 
    /* 0x39 */ EMPTY_TYPE,
    /* 0x3a */ EMPTY_TYPE,
    /* 0x3b */ EMPTY_TYPE,
    /* 0x3c */ EMPTY_TYPE, 
    /* 0x3d */ EMPTY_TYPE,
    /* 0x3e */ EMPTY_TYPE,
    /* 0x3f */ EMPTY_TYPE,

    /* 0x40 */ EMPTY_TYPE, 
    /* 0x41 */ EMPTY_TYPE,
    /* 0x42 */ EMPTY_TYPE,
    /* 0x43 */ EMPTY_TYPE,
    /* 0x44 */ EMPTY_TYPE, 
    /* 0x45 */ EMPTY_TYPE,
    /* 0x46 */ EMPTY_TYPE,
    /* 0x47 */ EMPTY_TYPE,
    /* 0x48 */ EMPTY_TYPE, 
    /* 0x49 */ EMPTY_TYPE,
    /* 0x4a */ EMPTY_TYPE,
    /* 0x4b */ EMPTY_TYPE,
    /* 0x4c */ EMPTY_TYPE, 
    /* 0x4d */ EMPTY_TYPE,
    /* 0x4e */ EMPTY_TYPE,
    /* 0x4f */ EMPTY_TYPE,

    /* 0x50 */ EMPTY_TYPE, 
    /* 0x51 */ EMPTY_TYPE,
    /* 0x52 */ EMPTY_TYPE,
    /* 0x53 */ EMPTY_TYPE,
    /* 0x54 */ EMPTY_TYPE, 
    /* 0x55 */ EMPTY_TYPE,
    /* 0x56 */ EMPTY_TYPE,
    /* 0x57 */ EMPTY_TYPE,
    /* 0x58 */ EMPTY_TYPE, 
    /* 0x59 */ EMPTY_TYPE,
    /* 0x5a */ EMPTY_TYPE,
    /* 0x5b */ EMPTY_TYPE,
    /* 0x5c */ EMPTY_TYPE, 
    /* 0x5d */ EMPTY_TYPE,
    /* 0x5e */ EMPTY_TYPE,
    /* 0x5f */ EMPTY_TYPE,

    /* 0x60 */ EMPTY_TYPE, 
    /* 0x61 */ EMPTY_TYPE,
    /* 0x62 */ EMPTY_TYPE,
    /* 0x63 */ EMPTY_TYPE,
    /* 0x64 */ EMPTY_TYPE, 
    /* 0x65 */ EMPTY_TYPE,
    /* 0x66 */ EMPTY_TYPE,
    /* 0x67 */ EMPTY_TYPE,
    /* 0x68 */ EMPTY_TYPE, 
    /* 0x69 */ EMPTY_TYPE,
    /* 0x6a */ EMPTY_TYPE,
    /* 0x6b */ EMPTY_TYPE,
    /* 0x6c */ EMPTY_TYPE, 
    /* 0x6d */ EMPTY_TYPE,
    /* 0x6e */ EMPTY_TYPE,
    /* 0x6f */ EMPTY_TYPE,

    /* 0x70 */ EMPTY_TYPE, 
    /* 0x71 */ EMPTY_TYPE,
    /* 0x72 */ EMPTY_TYPE,
    /* 0x73 */ EMPTY_TYPE,
    /* 0x74 */ EMPTY_TYPE, 
    /* 0x75 */ EMPTY_TYPE,
    /* 0x76 */ EMPTY_TYPE,
    /* 0x77 */ EMPTY_TYPE,
    /* 0x78 */ EMPTY_TYPE, 
    /* 0x79 */ EMPTY_TYPE,
    /* 0x7a */ EMPTY_TYPE,
    /* 0x7b */ EMPTY_TYPE,
    /* 0x7c */ EMPTY_TYPE, 
    /* 0x7d */ EMPTY_TYPE,
    /* 0x7e */ EMPTY_TYPE,
    /* 0x7f */ EMPTY_TYPE,

    /* 0x80 */ EMPTY_TYPE, 
    /* 0x81 */ EMPTY_TYPE,
    /* 0x82 */ EMPTY_TYPE,
    /* 0x83 */ EMPTY_TYPE,
    /* 0x84 */ EMPTY_TYPE, 
    /* 0x85 */ EMPTY_TYPE,
    /* 0x86 */ EMPTY_TYPE,
    /* 0x87 */ EMPTY_TYPE,
    /* 0x88 */ EMPTY_TYPE, 
    /* 0x89 */ EMPTY_TYPE,
    /* 0x8a */ EMPTY_TYPE,
    /* 0x8b */ EMPTY_TYPE,
    /* 0x8c */ EMPTY_TYPE, 
    /* 0x8d */ EMPTY_TYPE,
    /* 0x8e */ EMPTY_TYPE,
    /* 0x8f */ EMPTY_TYPE,

    /* 0xa0 */ EMPTY_TYPE, 
    /* 0xa1 */ EMPTY_TYPE,
    /* 0xa2 */ EMPTY_TYPE,
    /* 0xa3 */ EMPTY_TYPE,
    /* 0xa4 */ EMPTY_TYPE, 
    /* 0xa5 */ EMPTY_TYPE,
    /* 0xa6 */ EMPTY_TYPE,
    /* 0xa7 */ EMPTY_TYPE,
    /* 0xa8 */ EMPTY_TYPE, 
    /* 0xa9 */ EMPTY_TYPE,
    /* 0xaa */ EMPTY_TYPE,
    /* 0xab */ EMPTY_TYPE,
    /* 0xac */ EMPTY_TYPE, 
    /* 0xad */ EMPTY_TYPE,
    /* 0xae */ EMPTY_TYPE,
    /* 0xaf */ EMPTY_TYPE,

    /* 0xb0 */ EMPTY_TYPE, 
    /* 0xb1 */ EMPTY_TYPE,
    /* 0xb2 */ EMPTY_TYPE,
    /* 0xb3 */ EMPTY_TYPE,
    /* 0xb4 */ EMPTY_TYPE, 
    /* 0xb5 */ EMPTY_TYPE,
    /* 0xb6 */ EMPTY_TYPE,
    /* 0xb7 */ EMPTY_TYPE,
    /* 0xb8 */ EMPTY_TYPE, 
    /* 0xb9 */ EMPTY_TYPE,
    /* 0xba */ EMPTY_TYPE,
    /* 0xbb */ EMPTY_TYPE,
    /* 0xbc */ EMPTY_TYPE, 
    /* 0xbd */ EMPTY_TYPE,
    /* 0xbe */ EMPTY_TYPE,
    /* 0xbf */ EMPTY_TYPE,

    /* 0xc0 */ EMPTY_TYPE, 
    /* 0xc1 */ EMPTY_TYPE,
    /* 0xc2 */ EMPTY_TYPE,
    /* 0xc3 */ EMPTY_TYPE,
    /* 0xc4 */ EMPTY_TYPE, 
    /* 0xc5 */ EMPTY_TYPE,
    /* 0xc6 */ EMPTY_TYPE,
    /* 0xc7 */ EMPTY_TYPE,
    /* 0xc8 */ EMPTY_TYPE, 
    /* 0xc9 */ EMPTY_TYPE,
    /* 0xca */ EMPTY_TYPE,
    /* 0xcb */ EMPTY_TYPE,
    /* 0xcc */ EMPTY_TYPE, 
    /* 0xcd */ EMPTY_TYPE,
    /* 0xce */ EMPTY_TYPE,
    /* 0xcf */ EMPTY_TYPE,

    /* 0xd0 */ EMPTY_TYPE, 
    /* 0xd1 */ EMPTY_TYPE,
    /* 0xd2 */ EMPTY_TYPE,
    /* 0xd3 */ EMPTY_TYPE,
    /* 0xd4 */ EMPTY_TYPE, 
    /* 0xd5 */ EMPTY_TYPE,
    /* 0xd6 */ EMPTY_TYPE,
    /* 0xd7 */ EMPTY_TYPE,
    /* 0xd8 */ EMPTY_TYPE, 
    /* 0xd9 */ EMPTY_TYPE,
    /* 0xda */ EMPTY_TYPE,
    /* 0xdb */ EMPTY_TYPE,
    /* 0xdc */ EMPTY_TYPE, 
    /* 0xdd */ EMPTY_TYPE,
    /* 0xde */ EMPTY_TYPE,
    /* 0xdf */ EMPTY_TYPE,

    /* 0xe0 */ EMPTY_TYPE, 
    /* 0xe1 */ EMPTY_TYPE,
    /* 0xe2 */ EMPTY_TYPE,
    /* 0xe3 */ EMPTY_TYPE,
    /* 0xe4 */ EMPTY_TYPE, 
    /* 0xe5 */ EMPTY_TYPE,
    /* 0xe6 */ EMPTY_TYPE,
    /* 0xe7 */ EMPTY_TYPE,
    /* 0xe8 */ EMPTY_TYPE, 
    /* 0xe9 */ EMPTY_TYPE,
    /* 0xea */ EMPTY_TYPE,
    /* 0xeb */ EMPTY_TYPE,
    /* 0xec */ EMPTY_TYPE, 
    /* 0xed */ EMPTY_TYPE,
    /* 0xee */ EMPTY_TYPE,
    /* 0xef */ EMPTY_TYPE,

    /* 0xf0 */ EMPTY_TYPE, 
    /* 0xf1 */ EMPTY_TYPE,
    /* 0xf2 */ EMPTY_TYPE,
    /* 0xf3 */ EMPTY_TYPE,
    /* 0xf4 */ EMPTY_TYPE, 
    /* 0xf5 */ EMPTY_TYPE,
    /* 0xf6 */ EMPTY_TYPE,
    /* 0xf7 */ EMPTY_TYPE,
    /* 0xf8 */ EMPTY_TYPE, 
    /* 0xf9 */ EMPTY_TYPE,
    /* 0xfa */ EMPTY_TYPE,
    /* 0xfb */ EMPTY_TYPE,
    /* 0xfc */ EMPTY_TYPE, 
    /* 0xfd */ EMPTY_TYPE,
    /* 0xfe */ EMPTY_TYPE,
    /* 0xff */ EMPTY_TYPE
  };
