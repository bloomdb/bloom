#ifndef HASH64_H
#define HASH64_H

#include <stddef.h>
#include <stdint.h>

uint64_t hash64(const void* key, size_t len, uint64_t seed);

#endif
