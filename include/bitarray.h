#ifndef BITARRAY_H
#define BITARRAY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void bitarray_set(uint8_t* arr, size_t bit);
bool bitarray_get(const uint8_t* arr, size_t bit);

#endif
