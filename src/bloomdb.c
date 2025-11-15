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
// API PÚBLICA - Extended (error explícito)
// ============================================================================

const char* bloomdb_strerror(BloomDBError err) {
    switch (err) {
        case BLOOMDB_OK:
            return "Success";
        case BLOOMDB_ERR_INVALID_ARGUMENT:
            return "Invalid argument";
        case BLOOMDB_ERR_ALLOC:
            return "Memory allocation failed";
        case BLOOMDB_ERR_FILE_IO:
            return "File I/O error";
        case BLOOMDB_ERR_FORMAT:
            return "Invalid file format";
        case BLOOMDB_ERR_INTERNAL:
            return "Internal error";
        default:
            return "Unknown error";
    }
}

BloomDBError bloomdb_create_ex(size_t bits, int num_hashes, uint64_t seed, BloomDB** out_db) {
    if (!out_db) return BLOOMDB_ERR_INVALID_ARGUMENT;
    if (bits == 0 || num_hashes <= 0) return BLOOMDB_ERR_INVALID_ARGUMENT;

    BloomDB* db = malloc(sizeof(BloomDB));
    if (!db) return BLOOMDB_ERR_ALLOC;

    db->bit_count = bits;
    db->byte_count = (bits + 7) / 8;
    db->num_hashes = num_hashes;
    db->seed = seed;

    db->bitarray = calloc(db->byte_count, 1);
    if (!db->bitarray) {
        free(db);
        return BLOOMDB_ERR_ALLOC;
    }

    *out_db = db;
    return BLOOMDB_OK;
}

BloomDBError bloomdb_insert_ex(BloomDB* db, const void* key, size_t len) {
    if (!db || !key || len == 0) return BLOOMDB_ERR_INVALID_ARGUMENT;

    for (int i = 0; i < db->num_hashes; i++) {
        size_t bit = get_bit_index(db, key, len, i);
        set_bit(db->bitarray, bit);
    }
    return BLOOMDB_OK;
}

BloomDBError bloomdb_might_contain_ex(const BloomDB* db, const void* key, size_t len, bool* out_result) {
    if (!db || !key || len == 0 || !out_result) return BLOOMDB_ERR_INVALID_ARGUMENT;

    for (int i = 0; i < db->num_hashes; i++) {
        size_t bit = get_bit_index(db, key, len, i);
        if (!get_bit(db->bitarray, bit)) {
            *out_result = false;
            return BLOOMDB_OK;
        }
    }
    *out_result = true;
    return BLOOMDB_OK;
}

// ============================================================================
// API PÚBLICA - Simple (wrappers sobre _ex)
// ============================================================================

BloomDB* bloomdb_create(size_t bits, int num_hashes, uint64_t seed) {
    BloomDB* db = NULL;
    if (bloomdb_create_ex(bits, num_hashes, seed, &db) != BLOOMDB_OK) {
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
    return bloomdb_insert_ex(db, key, len) == BLOOMDB_OK;
}

bool bloomdb_might_contain(const BloomDB* db, const void* key, size_t len) {
    bool result = false;
    if (bloomdb_might_contain_ex(db, key, len, &result) != BLOOMDB_OK) {
        return false;
    }
    return result;
}
