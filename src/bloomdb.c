#include "bloomdb.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// INTERNAS (no forman parte de la API).
// ============================================================================

/**
 * Hash simple de 64 bits.
 *
 * Nota:
 * - Este hash NO ES seguro criptográficamente.
 * - El objetivo es velocidad y buena distribución para Bloom Filters.
 * - Más adelante mi idea es reemplazarlo por xxHash o Murmur3.
 */
static uint64_t hash_function(const void* key, size_t len, uint64_t seed) {
    const uint8_t* data = (const uint8_t*)key;
    uint64_t hash = seed;

    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x5bd1e995;
        hash ^= hash >> 15;
    }
    return hash;
}

/**
 * Deriva el índice de bit para el hash número hash_num.
 * Implementa doble hashing: h(i) = h1 + i*h2.
 */
static size_t get_bit_index(const BloomDB* db, const void* key, size_t len, int hash_num) {
    uint64_t h1 = hash_function(key, len, db->seed);
    uint64_t h2 = hash_function(key, len, db->seed + hash_num + 1);
    return (h1 + (uint64_t)hash_num * h2) % db->bit_count;
}

// Manipulación de bits (estas funciones son intencionalmente pequeñas)
static inline void set_bit(uint8_t* arr, size_t bit) {
    arr[bit >> 3] |= (1 << (bit & 7));
}

static inline bool get_bit(const uint8_t* arr, size_t bit) {
    return (arr[bit >> 3] & (1 << (bit & 7))) != 0;
}

// ============================================================================
// API PÚBLICA
// ============================================================================

BloomDB* bloomdb_create(size_t bits, int num_hashes, uint64_t seed) {
    if (bits == 0 || num_hashes <= 0) return NULL;

    BloomDB* db = malloc(sizeof(BloomDB));
    if (!db) return NULL;

    db->bit_count = bits;
    db->byte_count = (bits + 7) / 8;
    db->num_hashes = num_hashes;
    db->seed = seed;

    db->bitarray = calloc(db->byte_count, 1);
    if (!db->bitarray) {
        free(db);
        return NULL;
    }

    return db;
}

void bloomdb_free(BloomDB* db) {
    if (!db) return;
    free(db->bitarray);
    free(db);
}

bool bloomdb_insert(BloomDB* db, const void* key, size_t len) {
    if (!db || !key || len == 0) return false;

    for (int i = 0; i < db->num_hashes; i++) {
        size_t bit = get_bit_index(db, key, len, i);
        set_bit(db->bitarray, bit);
    }
    return true;
}

bool bloomdb_might_contain(const BloomDB* db, const void* key, size_t len) {
    if (!db || !key || len == 0) return false;

    for (int i = 0; i < db->num_hashes; i++) {
        size_t bit = get_bit_index(db, key, len, i);
        if (!get_bit(db->bitarray, bit))
            return false; // falso negativo imposible, osea retorno enseguida
    }
    return true; // podría existir xd (posible falso positivo)
}
