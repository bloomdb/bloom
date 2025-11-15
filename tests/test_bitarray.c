#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "bitarray.h"

int main(void) {
    printf("== test_bitarray ==\n");

    uint8_t bits[2] = {0, 0}; // 16 bits: 0..15

    // Al inicio todo debe estar en 0
    for (size_t i = 0; i < 16; i++) {
        assert(bitarray_get(bits, i) == false);
    }

    // Setear algunos bits específicos
    bitarray_set(bits, 0);   // primer bit
    bitarray_set(bits, 5);   // dentro del primer byte
    bitarray_set(bits, 8);   // primer bit del segundo byte
    bitarray_set(bits, 13);  // bit en el segundo byte

    // Verificar que están seteados
    assert(bitarray_get(bits, 0) == true);
    assert(bitarray_get(bits, 5) == true);
    assert(bitarray_get(bits, 8) == true);
    assert(bitarray_get(bits, 13) == true);

    // Verificar que otros bits siguen apagados
    assert(bitarray_get(bits, 1) == false);
    assert(bitarray_get(bits, 2) == false);
    assert(bitarray_get(bits, 7) == false);
    assert(bitarray_get(bits, 9) == false);
    assert(bitarray_get(bits, 15) == false);

    // Idempotencia: setear dos veces no debería romper nada
    bitarray_set(bits, 5);
    assert(bitarray_get(bits, 5) == true);

    printf("✓ test_bitarray: OK\n");
    return 0;
}
