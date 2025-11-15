#include "bitarray.h"

void bitarray_set(uint8_t* arr, size_t bit) {
    arr[bit >> 3] |= (1u << (bit & 7));
}

bool bitarray_get(const uint8_t* arr, size_t bit) {
    return (arr[bit >> 3] & (1u << (bit & 7))) != 0;
}
