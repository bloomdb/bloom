#include <stdio.h>
#include <string.h>
#include "bloomdb.h"
#include "storage.h"

int main(void) {
    BloomDB* db = bloomdb_create(10000, 5, 12345);
    if (!db) {
        printf("Error creando BloomDB\n");
        return 1;
    }

    const char* s1 = "hola";
    const char* s2 = "mundo";
    const char* s3 = "otro";

    bloomdb_insert(db, s1, strlen(s1));
    bloomdb_insert(db, s2, strlen(s2));

    printf("'hola'   -> %d\n", bloomdb_might_contain(db, s1, strlen(s1)));
    printf("'mundo'  -> %d\n", bloomdb_might_contain(db, s2, strlen(s2)));
    printf("'otro'   -> %d\n", bloomdb_might_contain(db, s3, strlen(s3)));

    bloomdb_save(db, "test.bloomdb");
    bloomdb_free(db);

    BloomDB* db2 = bloomdb_load("test.bloomdb");
    if (!db2) {
        printf("Error cargando BloomDB\n");
        return 1;
    }

    printf("Después de load:\n");
    printf("'hola'   -> %d\n", bloomdb_might_contain(db2, s1, strlen(s1)));
    printf("'mundo'  -> %d\n", bloomdb_might_contain(db2, s2, strlen(s2)));
    printf("'otro'   -> %d\n", bloomdb_might_contain(db2, s3, strlen(s3)));

    bloomdb_free(db2);
    return 0;
}
