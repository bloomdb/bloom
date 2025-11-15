# Benchmarking Framework

## Overview

Phase 1 includes a production-grade benchmarking framework that provides accurate, reproducible performance measurements comparable to industry-standard database systems.

## Design Philosophy

The benchmark framework follows methodologies used by:
- Redis (redis-benchmark)
- RocksDB (db_bench)
- SQLite (speedtest1)
- Google Benchmark
- ClickHouse performance suite

Key principles:
- High-precision measurements (nanosecond resolution)
- Statistical analysis with percentiles
- Warmup to eliminate cold-start effects
- CPU affinity for measurement stability
- Structured result export

## Framework Components

### Time Measurement

**Implementation**:
```c
static inline uint64_t ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
```

**Properties**:
- Uses CLOCK_MONOTONIC_RAW for stability
- Not affected by NTP adjustments
- Not affected by system clock changes
- Nanosecond resolution
- Minimal overhead (< 50ns per call)

**Alternative Considered**:
- CLOCK_MONOTONIC: Affected by NTP adjustments
- gettimeofday(): Microsecond resolution only
- RDTSC: CPU-specific, harder to use correctly

### CPU Pinning

**Purpose**: Reduce measurement variance by preventing thread migration

**Implementation**:
```c
static void pin_cpu() {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof(set), &set);
}
```

**Benefits**:
- Prevents context switches to other cores
- Maintains L1/L2 cache warmth
- Reduces measurement jitter
- More reproducible results

**Note**: Linux-specific. No-op on other platforms.

### Warmup Phase

**Purpose**: Eliminate cold-start effects before measurement

**Implementation**:
- Executes operation 2,000,000 times before timing
- Warms instruction cache
- Warms data cache
- Trains branch predictor
- Populates TLB

**Effects**:
- First measurement more representative
- Reduces variance in early runs
- Reflects steady-state performance

### Statistical Analysis

**Methodology**:
- 50 independent runs
- 1,000,000 operations per run
- Total: 50,000,000 operations measured

**Metrics Computed**:

1. **Average (Mean)**
   - Sum of all measurements / count
   - Gives overall performance sense
   - Can be skewed by outliers

2. **P50 (Median)**
   - 50th percentile
   - Middle value when sorted
   - Not affected by outliers
   - "Typical" latency

3. **P90**
   - 90th percentile
   - 90% of operations are faster
   - Useful for SLA definitions
   - Captures most user experience

4. **P99**
   - 99th percentile
   - Only 1% of operations are slower
   - Critical for tail latency
   - Detects performance spikes

**Calculation**:
```c
qsort(times, count, sizeof(uint64_t), cmp_u64);
uint64_t p50 = times[(int)(count * 0.50)];
uint64_t p90 = times[(int)(count * 0.90)];
uint64_t p99 = times[(int)(count * 0.99)];
```

## Benchmark Implementations

### bitarray_set

**Purpose**: Measure raw bit manipulation performance

**Setup**:
- 4096-byte array (32,768 bits)
- Sequential access pattern with wrapping
- Mask with `& 32767` to stay in bounds

**Measurement**:
- Time 1M set operations per run
- 50 runs total
- Calculate per-operation latency

**Expected Results**:
- Sub-nanosecond on modern CPUs
- Limited by L1 cache latency
- May show 0 ns on some runs due to rounding

**Optimization Opportunities**:
- Branchless implementation
- SIMD for batch operations
- Cache line alignment

### hash64

**Purpose**: Measure hash function performance

**Setup**:
- Fixed input: "hello_world_123" (15 bytes)
- Varying seed per iteration
- No memory allocation

**Measurement**:
- Time 1M hash operations per run
- 50 runs total
- Calculate per-operation latency

**Expected Results**:
- 8-15 ns/op range typical
- Dominated by multiplication and XOR operations
- Memory bandwidth not a factor (small input)

**Comparison Baseline**:
- xxHash64: ~5-8 ns/op
- MurmurHash3: ~6-10 ns/op
- CRC32: ~3-5 ns/op (hardware accelerated)

### bloomdb_insert

**Purpose**: Measure complete insert operation

**Setup**:
- 1,048,576 bits (1MB filter)
- 5 hash functions
- Seed: 123456
- String keys: "key0", "key1", etc.

**Operations per Insert**:
1. snprintf to format key string
2. strlen to get key length
3. 5 calls to hash function (double hashing)
4. 5 calls to bitarray_set
5. Modulo operations for index calculation

**Measurement**:
- Time 1M insert operations per run
- 50 runs total
- Calculate per-operation latency

**Expected Results**:
- 150-250 ns/op range typical
- Breakdown:
  - snprintf: ~50-80 ns
  - hash operations: ~60 ns (5 × 12ns)
  - bitarray operations: ~2-5 ns (5 × 0.4-1ns)
  - Other overhead: ~40-100 ns

**Optimization Opportunities**:
- Pre-format keys (remove snprintf)
- Cache hash values
- Batch operations
- Better hash function (xxHash)

## Result Export

### JSON Format

**File**: `benchmark_results.json`

**Structure**:
```json
{
  "benchmark_name": {
    "avg": 193.56,
    "p50": 189,
    "p90": 207,
    "p99": 251
  },
  ...
}
```

**Benefits**:
- Machine-readable
- Easy to parse with scripts
- Version control friendly
- Can be imported into analysis tools

### Processing Results

**Python Example**:
```python
import json
import matplotlib.pyplot as plt

with open('benchmark_results.json') as f:
    data = json.load(f)

benchmarks = list(data.keys())
p50_values = [data[b]['p50'] for b in benchmarks]

plt.bar(benchmarks, p50_values)
plt.ylabel('Latency (ns)')
plt.title('BloomDB Performance')
plt.savefig('benchmark.png')
```

**Comparison Script**:
```bash
# Compare two versions
diff <(jq . results_v1.json) <(jq . results_v2.json)
```

## Compilation

**Flags**:
```makefile
gcc -O3 -march=native -Iinclude src/*.c tests/benchmark_pro.c -o tests/benchmark_pro -lm
```

**Explanation**:
- `-O3`: Maximum optimization level
- `-march=native`: Use CPU-specific instructions
- `-lm`: Link math library (for qsort if needed)

**Impact**:
- `-O3` vs `-O0`: 3-10x speedup
- `-march=native`: 5-20% additional speedup
- Inlining, loop unrolling, vectorization enabled

## Running Benchmarks

**Command**:
```bash
make benchmark
```

**Output**:
```
Running BloomDB PRO Benchmark Suite

=== bitarray_set ===
Average: 0.40 ns/op
P50:     0 ns/op
P90:     1 ns/op
P99:     2 ns/op

=== hash64 ===
Average: 11.74 ns/op
P50:     12 ns/op
P90:     13 ns/op
P99:     15 ns/op

=== bloomdb_insert ===
Average: 193.56 ns/op
P50:     189 ns/op
P90:     207 ns/op
P99:     251 ns/op

Results exported to: benchmark_results.json
```

## Interpreting Results

### What to Look For

**Consistency**: P50, P90, P99 should be close
- Large gaps indicate variance
- P99 much higher than P50 suggests tail latency issues

**Performance**: Compare to expectations
- bitarray_set: Should be sub-nanosecond
- hash64: Should be 8-15 ns
- bloomdb_insert: Should be under 300 ns

**Regressions**: Compare across versions
- More than 10% slowdown is significant
- Less than 5% may be measurement noise

### Common Issues

**High Variance**:
- Other processes running
- Thermal throttling
- Power management
- Swap activity

**Solutions**:
- Run in isolated environment (Docker)
- Disable turbo boost for consistency
- Use dedicated benchmark machine
- Run multiple times and take minimum

## Benchmarking Best Practices

### Do's
- Run on idle system
- Run multiple times
- Look at percentiles, not just average
- Export and version control results
- Compare apples-to-apples (same machine, same compiler)

### Don'ts
- Don't benchmark in debug mode
- Don't run while other work is happening
- Don't ignore tail latencies
- Don't change too many things at once
- Don't trust a single run

## Future Enhancements

Planned additions:
- bloomdb_might_contain benchmark
- bloomdb_save/load benchmarks
- Variable filter size tests
- Variable number of hash functions
- Memory bandwidth measurement
- Cache miss profiling
- Comparison with other implementations

## Integration with Optimization

**Workflow**:
1. Establish baseline: `make benchmark > baseline.txt`
2. Make optimization changes
3. Re-run: `make benchmark > optimized.txt`
4. Compare: `diff baseline.txt optimized.txt`
5. Keep changes if improvement > 5%
6. Revert if regression or < 2% improvement

**Example Optimizations to Try**:
- Branchless bitarray_set
- xxHash replacement
- Remove snprintf from benchmark loop
- Prefetch bit array locations
- Batch operations

Each can be measured independently to determine actual impact.
