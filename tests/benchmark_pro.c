#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#ifdef __linux__
#include <sched.h>
#endif

#include "bitarray.h"
#include "hash64.h"
#include "bloomdb.h"
#include "storage.h"

#define RUNS 50        // número de repeticiones por test
#define N_OPS 1000000  // 1 millón de operaciones por run

// =========================================================
//  UTILIDADES PRO DE BENCHMARK
// =========================================================

static inline uint64_t ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void pin_cpu() {
#ifdef __linux__
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof(set), &set);
#endif
}

// =========================================================
//  ESTADÍSTICAS: percentiles + promedio
// =========================================================

static int cmp_u64(const void* a, const void* b) {
    uint64_t x = *(uint64_t*)a;
    uint64_t y = *(uint64_t*)b;
    return (x > y) - (x < y);
}

static void compute_stats(uint64_t times[], int count,
                          const char* label, FILE* json) {

    qsort(times, count, sizeof(uint64_t), cmp_u64);

    double avg = 0;
    for (int i = 0; i < count; i++) avg += times[i];
    avg /= count;

    uint64_t p50 = times[(int)(count * 0.50)];
    uint64_t p90 = times[(int)(count * 0.90)];
    uint64_t p99 = times[(int)(count * 0.99)];

    printf("\n=== %s ===\n", label);
    printf("Average: %.2f ns/op\n", avg);
    printf("P50:     %lu ns/op\n", (unsigned long)p50);
    printf("P90:     %lu ns/op\n", (unsigned long)p90);
    printf("P99:     %lu ns/op\n", (unsigned long)p99);

    // exportar a JSON
    if (json) {
        fprintf(json,
            "  \"%s\": {\n"
            "    \"avg\": %.2f,\n"
            "    \"p50\": %lu,\n"
            "    \"p90\": %lu,\n"
            "    \"p99\": %lu\n"
            "  },\n",
            label, avg, (unsigned long)p50, (unsigned long)p90, (unsigned long)p99
        );
    }
}

// =========================================================
//  TESTS PRO
// =========================================================

void bench_bitarray(FILE* json) {
    uint8_t arr[4096] = {0};
    uint64_t times[RUNS];

    // Warmup
    for (int i = 0; i < 2000000; i++)
        bitarray_set(arr, i & 32767);

    for (int r = 0; r < RUNS; r++) {
        uint64_t start = ns();
        for (int i = 0; i < N_OPS; i++)
            bitarray_set(arr, i & 32767);
        uint64_t end = ns();
        times[r] = (end - start) / N_OPS;
    }

    compute_stats(times, RUNS, "bitarray_set", json);
}

void bench_hash64(FILE* json) {
    const char* s = "hello_world_123";
    uint64_t times[RUNS];

    // warmup
    for (int i = 0; i < 2000000; i++)
        hash64(s, 16, i);

    for (int r = 0; r < RUNS; r++) {
        uint64_t start = ns();
        for (int i = 0; i < N_OPS; i++)
            hash64(s, 16, i);
        uint64_t end = ns();
        times[r] = (end - start) / N_OPS;
    }

    compute_stats(times, RUNS, "hash64", json);
}

void bench_bloom_insert(FILE* json) {
    BloomDB* db = bloomdb_create(1 << 20, 5, 123456);
    uint64_t times[RUNS];

    // warmup
    for (int i = 0; i < 200000; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key%d", i);
        bloomdb_insert(db, buf, strlen(buf));
    }

    for (int r = 0; r < RUNS; r++) {
        uint64_t start = ns();
        for (int i = 0; i < N_OPS; i++) {
            char buf[32];
            snprintf(buf, sizeof(buf), "key%d", i);
            bloomdb_insert(db, buf, strlen(buf));
        }
        uint64_t end = ns();
        times[r] = (end - start) / N_OPS;
    }

    compute_stats(times, RUNS, "bloomdb_insert", json);

    bloomdb_free(db);
}

int main() {
    pin_cpu(); // fijar a un solo core para estabilidad

    printf("╔═══════════════════════════════════════════╗\n");
    printf("║   🔥 BloomDB PRO Benchmark Suite         ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    FILE* json = fopen("benchmark_results.json", "w");
    if (json) fprintf(json, "{\n");

    bench_bitarray(json);
    bench_hash64(json);
    bench_bloom_insert(json);

    if (json) {
        // Remove trailing comma from last entry
        fseek(json, -2, SEEK_CUR);
        fprintf(json, "\n}\n");
        fclose(json);
        printf("\n✅ Results exported to: benchmark_results.json\n");
    }

    return 0;
}
