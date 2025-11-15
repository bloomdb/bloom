#ifndef BLOOMDB_H
#define BLOOMDB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// ============================================================================
// Error Codes
// ============================================================================

typedef enum {
    BLOOMDB_OK = 0,
    BLOOMDB_ERR_INVALID_ARGUMENT,
    BLOOMDB_ERR_ALLOC,
    BLOOMDB_ERR_FILE_IO,
    BLOOMDB_ERR_FORMAT,
    BLOOMDB_ERR_INTERNAL
} BloomDBError;

const char* bloomdb_strerror(BloomDBError err);

// ============================================================================
// Core Data Structure
// ============================================================================

typedef struct {
    uint8_t* bitarray;   //arreglo de bits comprimido
    size_t bit_count;    //número de bits totales
    size_t byte_count;   //número de bytes usados
    int num_hashes;      //cantidad de hashes k
    uint64_t seed;       //semilla del hash
} BloomDB;

// ============================================================================
// Core API (Simple - returns NULL/false on error)
// ============================================================================

BloomDB* bloomdb_create(size_t bits, int num_hashes, uint64_t seed);
void bloomdb_free(BloomDB* db);
bool bloomdb_insert(BloomDB* db, const void* key, size_t len);
bool bloomdb_might_contain(const BloomDB* db, const void* key, size_t len);

// ============================================================================
// Extended API (Explicit error handling)
// ============================================================================

BloomDBError bloomdb_create_ex(size_t bits, int num_hashes, uint64_t seed, BloomDB** out_db);
BloomDBError bloomdb_insert_ex(BloomDB* db, const void* key, size_t len);
BloomDBError bloomdb_might_contain_ex(const BloomDB* db, const void* key, size_t len, bool* out_result);

// ============================================================================
// Helper Functions (C strings)
// ============================================================================

static inline bool bloomdb_insert_cstr(BloomDB* db, const char* s) {
    if (!s) return false;
    return bloomdb_insert(db, s, strlen(s));
}

static inline bool bloomdb_might_contain_cstr(const BloomDB* db, const char* s) {
    if (!s) return false;
    return bloomdb_might_contain(db, s, strlen(s));
}

static inline BloomDBError bloomdb_insert_cstr_ex(BloomDB* db, const char* s) {
    if (!s) return BLOOMDB_ERR_INVALID_ARGUMENT;
    return bloomdb_insert_ex(db, s, strlen(s));
}

static inline BloomDBError bloomdb_might_contain_cstr_ex(const BloomDB* db, const char* s, bool* out_result) {
    if (!s) return BLOOMDB_ERR_INVALID_ARGUMENT;
    return bloomdb_might_contain_ex(db, s, strlen(s), out_result);
}

// ============================================================================
// Helper Functions (uint64_t)
// ============================================================================

static inline bool bloomdb_insert_u64(BloomDB* db, uint64_t value) {
    return bloomdb_insert(db, &value, sizeof(value));
}

static inline bool bloomdb_might_contain_u64(const BloomDB* db, uint64_t value) {
    return bloomdb_might_contain(db, &value, sizeof(value));
}

static inline BloomDBError bloomdb_insert_u64_ex(BloomDB* db, uint64_t value) {
    return bloomdb_insert_ex(db, &value, sizeof(value));
}

static inline BloomDBError bloomdb_might_contain_u64_ex(const BloomDB* db, uint64_t value, bool* out_result) {
    return bloomdb_might_contain_ex(db, &value, sizeof(value), out_result);
}

#endif

