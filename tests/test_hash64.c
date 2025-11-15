#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "hash64.h"

static void test_deterministic(void) {
    const char* key = "hello";
    uint64_t h1 = hash64(key, strlen(key), 12345);
    uint64_t h2 = hash64(key, strlen(key), 12345);

    assert(h1 == h2); // misma entrada, mismo seed => mismo hash
}

static void test_different_keys(void) {
    const char* a = "a";
    const char* b = "b";
    const char* c = "abc";

    uint64_t ha = hash64(a, strlen(a), 1);
    uint64_t hb = hash64(b, strlen(b), 1);
    uint64_t hc = hash64(c, strlen(c), 1);

    // Es extremadamente improbable que alguno coincida
    assert(ha != hb);
    assert(ha != hc);
    assert(hb != hc);
}

static void test_different_seeds(void) {
    const char* key = "same-key";

    uint64_t h1 = hash64(key, strlen(key), 1);
    uint64_t h2 = hash64(key, strlen(key), 2);
    uint64_t h3 = hash64(key, strlen(key), 9999);

    // Distintas seeds deberían generar hashes distintos
    assert(h1 != h2);
    assert(h1 != h3);
    assert(h2 != h3);
}

int main(void) {
    printf("== test_hash64 ==\n");

    test_deterministic();
    test_different_keys();
    test_different_seeds();

    printf("✓ test_hash64: OK\n");
    return 0;
}
