#include "storage.h"
#include "bloomdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

bool bloomdb_save(const BloomDB* db, const char* path) {
    if (!db || !path) return false;

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    fwrite(&db->bit_count,  sizeof(size_t), 1, f);
    fwrite(&db->byte_count, sizeof(size_t), 1, f);
    fwrite(&db->num_hashes, sizeof(int),    1, f);
    fwrite(&db->seed,       sizeof(uint64_t), 1, f);
    fwrite(db->bitarray, 1, db->byte_count, f);

    fclose(f);
    return true;
}

BloomDB* bloomdb_load(const char* path) {
    if (!path) return NULL;

    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    size_t bits, bytes;
    int num_hashes;
    uint64_t seed;

    if (fread(&bits, sizeof(size_t), 1, f) != 1 ||
        fread(&bytes, sizeof(size_t), 1, f) != 1 ||
        fread(&num_hashes, sizeof(int), 1, f) != 1 ||
        fread(&seed, sizeof(uint64_t), 1, f) != 1) {
        fclose(f);
        return NULL;
    }

    BloomDB* db = bloomdb_create(bits, num_hashes, seed);
    if (!db) {
        fclose(f);
        return NULL;
    }

    if (fread(db->bitarray, 1, bytes, f) != bytes) {
        bloomdb_free(db);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return db;
}
