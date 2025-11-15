#include "storage.h"
#include "bloomdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

// ============================================================================
// Extended API (explicit error handling)
// ============================================================================

BloomDBError bloomdb_save_ex(const BloomDB* db, const char* path) {
    if (!db || !path) return BLOOMDB_ERR_INVALID_ARGUMENT;

    FILE* f = fopen(path, "wb");
    if (!f) return BLOOMDB_ERR_FILE_IO;

    size_t written = 0;
    written += fwrite(&db->bit_count,  sizeof(size_t), 1, f);
    written += fwrite(&db->byte_count, sizeof(size_t), 1, f);
    written += fwrite(&db->num_hashes, sizeof(int),    1, f);
    written += fwrite(&db->seed,       sizeof(uint64_t), 1, f);
    
    if (written != 4) {
        fclose(f);
        return BLOOMDB_ERR_FILE_IO;
    }

    if (fwrite(db->bitarray, 1, db->byte_count, f) != db->byte_count) {
        fclose(f);
        return BLOOMDB_ERR_FILE_IO;
    }

    fclose(f);
    return BLOOMDB_OK;
}

BloomDBError bloomdb_load_ex(const char* path, BloomDB** out_db) {
    if (!path || !out_db) return BLOOMDB_ERR_INVALID_ARGUMENT;

    FILE* f = fopen(path, "rb");
    if (!f) return BLOOMDB_ERR_FILE_IO;

    size_t bits, bytes;
    int num_hashes;
    uint64_t seed;

    if (fread(&bits, sizeof(size_t), 1, f) != 1 ||
        fread(&bytes, sizeof(size_t), 1, f) != 1 ||
        fread(&num_hashes, sizeof(int), 1, f) != 1 ||
        fread(&seed, sizeof(uint64_t), 1, f) != 1) {
        fclose(f);
        return BLOOMDB_ERR_FORMAT;
    }

    // Validar valores razonables
    if (bits == 0 || num_hashes <= 0 || bytes != (bits + 7) / 8) {
        fclose(f);
        return BLOOMDB_ERR_FORMAT;
    }

    BloomDB* db = NULL;
    BloomDBError err = bloomdb_create_ex(bits, num_hashes, seed, &db);
    if (err != BLOOMDB_OK) {
        fclose(f);
        return err;
    }

    if (fread(db->bitarray, 1, bytes, f) != bytes) {
        bloomdb_free(db);
        fclose(f);
        return BLOOMDB_ERR_FORMAT;
    }

    fclose(f);
    *out_db = db;
    return BLOOMDB_OK;
}

// ============================================================================
// Simple API (wrappers)
// ============================================================================

bool bloomdb_save(const BloomDB* db, const char* path) {
    return bloomdb_save_ex(db, path) == BLOOMDB_OK;
}

BloomDB* bloomdb_load(const char* path) {
    BloomDB* db = NULL;
    if (bloomdb_load_ex(path, &db) != BLOOMDB_OK) {
        return NULL;
    }
    return db;
}
