#ifndef MURMUR_H
#define MURMUR_H

#include <stdint.h>

/*
 * Calculate the Murmur3 hash of a given series of bytes. MurmurHash3 was written by
 * Austin Appleby, and is placed in the public domain. The author hereby disclaims
 * copyright to this source code.
 */
uint32_t murmur3_32(const char* key, uint32_t length, uint32_t seed);

#endif
