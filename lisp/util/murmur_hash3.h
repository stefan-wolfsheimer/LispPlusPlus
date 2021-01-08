#ifndef __MURMUR_HASH3_H___
#define __MURMUR_HASH3_H___
#include <stdint.h>

void murmur_hash3_x86_32(const void * key, 
                         int len, 
                         uint32_t seed, 
                         uint32_t * out );

void murmur_hash3_x86_128 (const void * key, 
                           int len, 
                           uint32_t seed, 
                           void * out );

void murmur_hash3_x64_128 ( const void * key, 
                            int len, 
                            uint32_t seed, 
                            void * out );

#endif
