# BloomDB

<img width="278" height="271" alt="logobloom" src="https://github.com/user-attachments/assets/05b13603-e59c-4200-98a2-958a86bb300b" />
<br>

BloomDB es un filtro de Bloom escrito en C, pensado para ser:

- muy rápido
- simple de integrar
- fácil de persistir a disco

Ideal para casos como:

- evitar buscar en disco/BD si ya sabemos que una clave NO está
- pre-filtrar emails, IDs, URLs, etc.
- experimentar con estructuras probabilísticas en C

---

## Características

- Implementado en C11 puro (sin dependencias externas)
- API pequeña: `create`, `insert`, `might_contain`, `save`, `load`
- Persistencia binaria a archivo (`.bloomdb`)
- Arquitectura modular: `bloomdb`, `bitarray`, `hash64`, `storage`

> **Nota:** Actualmente BloomDB **no es thread-safe**.  
> Se asume uso desde un único hilo. Soporte para concurrencia se evaluará en futuras versiones.

---

## Ejemplo rápido

```c
#include <stdio.h>
#include <string.h>
#include "bloomdb.h"
#include "storage.h"

int main(void) {
    BloomDB* db = bloomdb_create(10000, 5, 12345);
    if (!db) return 1;

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
    if (!db2) return 1;

    printf("Después de load:\n");
    printf("'hola'   -> %d\n", bloomdb_might_contain(db2, s1, strlen(s1)));
    printf("'mundo'  -> %d\n", bloomdb_might_contain(db2, s2, strlen(s2)));
    printf("'otro'   -> %d\n", bloomdb_might_contain(db2, s3, strlen(s3)));

    bloomdb_free(db2);
    return 0;
}

