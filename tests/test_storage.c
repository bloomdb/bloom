#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bloomdb.h"
#include "storage.h"

int main(void) {
    printf("== test_storage ==\n");

    const char* path = "test_filter.bloomdb";

    size_t bits = 50000;
    int num_hashes = 4;
    uint64_t seed = 1234;

    // 1) Crear filtro original
    BloomDB* db = bloomdb_create(bits, num_hashes, seed);
    assert(db != NULL);

    const char* inserted[] = {
        "foo",
        "bar",
        "baz",
        "qux"
    };
    const size_t inserted_count = sizeof(inserted) / sizeof(inserted[0]);

    for (size_t i = 0; i < inserted_count; i++) {
        bool ok = bloomdb_insert(db, inserted[i], strlen(inserted[i]));
        assert(ok == true);
    }

    // 2) Guardar a disco
    bool saved = bloomdb_save(db, path);
    assert(saved == true);

    // 3) Liberar filtro original
    bloomdb_free(db);

    // 4) Cargar de disco
    BloomDB* loaded = bloomdb_load(path);
    assert(loaded != NULL);

    // 5) Verificar metadata básica
    assert(loaded->bit_count == bits);
    assert(loaded->num_hashes == num_hashes);
    assert(loaded->seed == seed);
    assert(loaded->byte_count == (bits + 7) / 8);
    assert(loaded->bitarray != NULL);

    // 6) Verificar que las claves insertadas siguen "probablemente presentes"
    for (size_t i = 0; i < inserted_count; i++) {
        bool maybe = bloomdb_might_contain(loaded, inserted[i], strlen(inserted[i]));
        assert(maybe == true);
    }

    // 7) Clave que no insertamos debería dar false (casi siempre)
    bool maybe_unknown = bloomdb_might_contain(loaded, "non-existent-key", strlen("non-existent-key"));
    // Podría ser true por falso positivo, pero con pocos inserts y filtro grande casi siempre será false
    // Igual no hacemos assert aquí para evitar flaky tests, solo lo imprimimos.
    printf("   bloomdb_might_contain(\"non-existent-key\") = %s\n",
           maybe_unknown ? "true (posible falso positivo)" : "false (correcto)");

    bloomdb_free(loaded);

    // 8) Limpiar archivo de prueba (si te molesta dejarlo)
    remove(path);

    printf("✓ test_storage: OK\n");
    return 0;
}
