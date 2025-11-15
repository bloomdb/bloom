#include "hash64.h"

uint64_t hash64(const void* key, size_t len, uint64_t seed) {
    const uint8_t* data = (const uint8_t*)key;
    uint64_t hash = seed;

    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x5bd1e995;
        hash ^= hash >> 15;
    }
    return hash;
}
