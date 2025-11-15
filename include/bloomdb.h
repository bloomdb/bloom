#ifndef BLOOMDB_H
#define BLOOMDB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t* bitarray;   //arreglo de bits comprimido
    size_t bit_count;    //número de bits totales
    size_t byte_count;   //número de bytes usados
    int num_hashes;      //cantidad de hashes k
    uint64_t seed;       //semilla del hash
} BloomDB;

BloomDB* bloomdb_create(size_t bits, int num_hashes, uint64_t seed);
void bloomdb_free(BloomDB* db);
bool bloomdb_insert(BloomDB* db, const void* key, size_t len);
bool bloomdb_might_contain(const BloomDB* db, const void* key, size_t len);

#endif

