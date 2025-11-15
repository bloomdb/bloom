#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bloomdb.h"

int main(void) {
    printf("== test_bloomdb_ex ==\n");

    // Test 1: bloomdb_create_ex con argumentos inválidos
    BloomDB* db = NULL;
    BloomDBError err;

    // bits = 0
    err = bloomdb_create_ex(0, 3, 42, &db);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);
    assert(db == NULL);

    // num_hashes <= 0
    err = bloomdb_create_ex(1000, 0, 42, &db);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);
    assert(db == NULL);

    err = bloomdb_create_ex(1000, -5, 42, &db);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);
    assert(db == NULL);

    // out_db == NULL
    err = bloomdb_create_ex(1000, 3, 42, NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    // Test 2: bloomdb_create_ex válido
    err = bloomdb_create_ex(10000, 5, 42, &db);
    assert(err == BLOOMDB_OK);
    assert(db != NULL);
    assert(db->bit_count == 10000);
    assert(db->num_hashes == 5);
    assert(db->seed == 42);

    // Test 3: bloomdb_insert_ex con argumentos inválidos
    err = bloomdb_insert_ex(NULL, "key", 3);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_insert_ex(db, NULL, 3);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_insert_ex(db, "key", 0);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    // Test 4: bloomdb_insert_ex válido
    const char* test_key = "test_key";
    err = bloomdb_insert_ex(db, test_key, strlen(test_key));
    assert(err == BLOOMDB_OK);

    // Test 5: bloomdb_might_contain_ex con argumentos inválidos
    bool result;
    
    err = bloomdb_might_contain_ex(NULL, "key", 3, &result);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_ex(db, NULL, 3, &result);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_ex(db, "key", 0, &result);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_ex(db, "key", 3, NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    // Test 6: bloomdb_might_contain_ex válido
    err = bloomdb_might_contain_ex(db, test_key, strlen(test_key), &result);
    assert(err == BLOOMDB_OK);
    assert(result == true);  // La clave que insertamos debe estar

    const char* missing_key = "missing";
    err = bloomdb_might_contain_ex(db, missing_key, strlen(missing_key), &result);
    assert(err == BLOOMDB_OK);
    // result puede ser true o false (posible falso positivo)

    // Test 7: bloomdb_strerror
    assert(strcmp(bloomdb_strerror(BLOOMDB_OK), "Success") == 0);
    assert(strcmp(bloomdb_strerror(BLOOMDB_ERR_INVALID_ARGUMENT), "Invalid argument") == 0);
    assert(strcmp(bloomdb_strerror(BLOOMDB_ERR_ALLOC), "Memory allocation failed") == 0);
    assert(strcmp(bloomdb_strerror(BLOOMDB_ERR_FILE_IO), "File I/O error") == 0);
    assert(strcmp(bloomdb_strerror(BLOOMDB_ERR_FORMAT), "Invalid file format") == 0);

    bloomdb_free(db);

    printf("✓ test_bloomdb_ex: OK\n");
    return 0;
}
