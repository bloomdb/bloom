#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "bloomdb.h"
#include "storage.h"

int main(void) {
    printf("== test_storage_ex ==\n");

    // Test 1: bloomdb_save_ex con argumentos inválidos
    BloomDB* db = NULL;
    BloomDBError err;

    err = bloomdb_create_ex(1000, 3, 42, &db);
    assert(err == BLOOMDB_OK);
    assert(db != NULL);

    // db == NULL
    err = bloomdb_save_ex(NULL, "test.bloom");
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    // path == NULL
    err = bloomdb_save_ex(db, NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    // Test 2: bloomdb_save_ex válido
    const char* path = "test_ex.bloom";
    err = bloomdb_save_ex(db, path);
    assert(err == BLOOMDB_OK);

    // Test 3: bloomdb_load_ex con argumentos inválidos
    BloomDB* loaded = NULL;

    // path == NULL
    err = bloomdb_load_ex(NULL, &loaded);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);
    assert(loaded == NULL);

    // out_db == NULL
    err = bloomdb_load_ex(path, NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    // Test 4: bloomdb_load_ex con archivo inexistente
    err = bloomdb_load_ex("nonexistent.bloom", &loaded);
    assert(err == BLOOMDB_ERR_FILE_IO);
    assert(loaded == NULL);

    // Test 5: bloomdb_load_ex válido
    err = bloomdb_load_ex(path, &loaded);
    assert(err == BLOOMDB_OK);
    assert(loaded != NULL);
    assert(loaded->bit_count == db->bit_count);
    assert(loaded->num_hashes == db->num_hashes);
    assert(loaded->seed == db->seed);

    // Test 6: bloomdb_load_ex con formato corrupto
    // Crear archivo con datos inválidos (bits = 0)
    const char* corrupt_path = "test_corrupt.bloom";
    FILE* f = fopen(corrupt_path, "wb");
    assert(f != NULL);
    size_t zero_bits = 0;
    size_t zero_hashes = 0;
    size_t bad_seed = 123;
    size_t byte_count = 0;
    fwrite(&zero_bits, sizeof(size_t), 1, f);
    fwrite(&zero_hashes, sizeof(size_t), 1, f);
    fwrite(&bad_seed, sizeof(size_t), 1, f);
    fwrite(&byte_count, sizeof(size_t), 1, f);
    fclose(f);

    BloomDB* corrupt_db = NULL;
    err = bloomdb_load_ex(corrupt_path, &corrupt_db);
    assert(err == BLOOMDB_ERR_FORMAT);
    assert(corrupt_db == NULL);

    // Test 7: Archivo truncado (no se puede leer completamente)
    const char* truncated_path = "test_truncated.bloom";
    f = fopen(truncated_path, "wb");
    assert(f != NULL);
    fwrite(&zero_bits, sizeof(size_t), 1, f); // Solo escribimos 1 campo en vez de 4
    fclose(f);

    BloomDB* truncated_db = NULL;
    err = bloomdb_load_ex(truncated_path, &truncated_db);
    assert(err == BLOOMDB_ERR_FORMAT);  // Archivo truncado se considera formato inválido
    assert(truncated_db == NULL);

    // Cleanup
    bloomdb_free(db);
    bloomdb_free(loaded);
    unlink(path);
    unlink(corrupt_path);
    unlink(truncated_path);

    printf("✓ test_storage_ex: OK\n");
    return 0;
}
