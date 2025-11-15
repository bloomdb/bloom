#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bloomdb.h"

int main(void) {
    printf("== test_helpers ==\n");

    BloomDB* db = bloomdb_create(1000, 3, 42);
    assert(db != NULL);

    // Test 1: Helper functions cstr (insert and query with C strings)
    bool inserted = bloomdb_insert_cstr(db, "hello");
    assert(inserted == true);

    bool found = bloomdb_might_contain_cstr(db, "hello");
    assert(found == true);

    bool not_found = bloomdb_might_contain_cstr(db, "world");
    // not_found puede ser true (falso positivo) o false

    // Test 2: Helper functions cstr_ex (with explicit error handling)
    BloomDBError err = bloomdb_insert_cstr_ex(db, "test");
    assert(err == BLOOMDB_OK);

    bool result;
    err = bloomdb_might_contain_cstr_ex(db, "test", &result);
    assert(err == BLOOMDB_OK);
    assert(result == true);

    // Test 3: Helper functions u64 (insert and query uint64_t values)
    uint64_t val1 = 123456789ULL;
    bool inserted_u64 = bloomdb_insert_u64(db, val1);
    assert(inserted_u64 == true);

    bool found_u64 = bloomdb_might_contain_u64(db, val1);
    assert(found_u64 == true);

    uint64_t val2 = 987654321ULL;
    bool not_found_u64 = bloomdb_might_contain_u64(db, val2);
    // not_found_u64 puede ser true o false

    // Test 4: Helper functions u64_ex (with explicit error handling)
    err = bloomdb_insert_u64_ex(db, val2);
    assert(err == BLOOMDB_OK);

    err = bloomdb_might_contain_u64_ex(db, val2, &result);
    assert(err == BLOOMDB_OK);
    assert(result == true);

    // Test 5: Test error handling with NULL arguments
    err = bloomdb_insert_cstr_ex(NULL, "test");
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_insert_cstr_ex(db, NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_cstr_ex(NULL, "test", &result);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_cstr_ex(db, NULL, &result);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_cstr_ex(db, "test", NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_insert_u64_ex(NULL, 123);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_u64_ex(NULL, 123, &result);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    err = bloomdb_might_contain_u64_ex(db, 123, NULL);
    assert(err == BLOOMDB_ERR_INVALID_ARGUMENT);

    bloomdb_free(db);

    printf("✓ test_helpers: OK\n");
    return 0;
}

