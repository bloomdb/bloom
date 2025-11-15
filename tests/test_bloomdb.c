#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bloomdb.h"

int main(void) {
    printf("== test_bloomdb ==\n");

    // Filtro grande para minimizar falsos positivos en el test
    size_t bits = 100000;     // 100k bits
    int num_hashes = 5;
    uint64_t seed = 42;

    BloomDB* db = bloomdb_create(bits, num_hashes, seed);
    assert(db != NULL);

    const char* inserted[] = {
        "alice@example.com",
        "bob@example.com",
        "charlie@example.com",
        "david@example.com",
        "eve@example.com"
    };
    const size_t inserted_count = sizeof(inserted) / sizeof(inserted[0]);

    const char* not_inserted[] = {
        "mallory@example.com",
        "oscar@example.com",
        "peggy@example.com",
        "trent@example.com",
        "victor@example.com"
    };
    const size_t not_inserted_count = sizeof(not_inserted) / sizeof(not_inserted[0]);

    // Insertar claves
    for (size_t i = 0; i < inserted_count; i++) {
        bool ok = bloomdb_insert(db, inserted[i], strlen(inserted[i]));
        assert(ok == true);
    }

    // Verificar que TODAS las insertadas "probablemente estén"
    for (size_t i = 0; i < inserted_count; i++) {
        bool maybe = bloomdb_might_contain(db, inserted[i], strlen(inserted[i]));
        assert(maybe == true);
    }

    // Verificar que al menos una NO insertada devuelva false
    size_t definitely_not = 0;
    for (size_t i = 0; i < not_inserted_count; i++) {
        bool maybe = bloomdb_might_contain(db, not_inserted[i], strlen(not_inserted[i]));
        if (!maybe) {
            definitely_not++;
        }
    }
    assert(definitely_not >= 1); // si fuera 0, sería MUY raro → sospechoso

    bloomdb_free(db);

    printf("✓ test_bloomdb: OK\n");
    return 0;
}
