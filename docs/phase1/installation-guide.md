# Installation Guide - Phase 1

## Prerequisites

### Required
- Docker and Docker Compose
- Git
- 2GB of free disk space

### Optional (for native compilation)
- GCC 4.8+ or Clang 3.1+
- GNU Make
- Valgrind (for memory testing)

## Installation Methods

### Method 1: Docker (Recommended)

This is the easiest and most reliable method, providing a consistent environment regardless of your host OS.

#### Step 1: Clone the repository

```bash
git clone https://github.com/bloomdb/bloom.git
cd bloom
```

#### Step 2: Start the Docker container

```bash
docker-compose up -d
```

This will:
- Build an Ubuntu 22.04 container with all development tools
- Mount the project directory at `/app`
- Keep the container running in the background

#### Step 3: Verify installation

```bash
docker exec bloomdb-dev make build
docker exec bloomdb-dev ./bloomdb
```

You should see output demonstrating Bloom filter operations.

### Method 2: Native Compilation (Linux/macOS)

#### Step 1: Install dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential valgrind git
```

**macOS:**
```bash
brew install gcc make valgrind
```

#### Step 2: Clone and build

```bash
git clone https://github.com/bloomdb/bloom.git
cd bloom
make build
```

#### Step 3: Run

```bash
./bloomdb
```

### Method 3: Windows (WSL2)

#### Step 1: Enable WSL2

```powershell
wsl --install
```

#### Step 2: Install Ubuntu from Microsoft Store

#### Step 3: Follow Linux instructions

Inside WSL2 Ubuntu terminal, follow Method 2 instructions.

## Verifying Installation

### Quick Test

```bash
# In Docker
docker exec bloomdb-dev ./bloomdb

# Native
./bloomdb
```

Expected output:
```
'hola'   -> 1
'mundo'  -> 1
'otro'   -> 0
Después de load:
'hola'   -> 1
'mundo'  -> 1
'otro'   -> 0
```

### Run Test Suite

```bash
# In Docker
docker exec bloomdb-dev make test

# Native
make test
```

All tests should pass with "OK" status.

## What BloomDB Does (Phase 1)

BloomDB is a probabilistic data structure that allows you to test set membership with:
- Very low memory usage (bits per element)
- Constant-time operations
- No false negatives
- Tunable false positive rate

### Core Operations

#### 1. Create a Filter

```c
BloomDB* db = bloomdb_create(
    1000000,    // 1 million bits (~122 KB)
    5,          // 5 hash functions
    42          // seed for reproducibility
);
```

Parameters:
- `bits`: Total number of bits in the filter (controls false positive rate)
- `num_hashes`: Number of hash functions (optimal is `(m/n) * ln(2)`)
- `seed`: Random seed for hash functions

#### 2. Insert Elements

```c
const char* email = "user@example.com";
bloomdb_insert(db, email, strlen(email));
```

Inserts are:
- Constant time: O(k) where k is number of hash functions
- Irreversible: Cannot remove elements
- Idempotent: Inserting twice has no additional effect

#### 3. Query Membership

```c
if (bloomdb_might_contain(db, email, strlen(email))) {
    // Element might be in the set (or false positive)
} else {
    // Element is definitely NOT in the set
}
```

Query guarantees:
- If returns `false`: Element was definitely not inserted
- If returns `true`: Element was probably inserted (may be false positive)

#### 4. Persist to Disk

```c
bloomdb_save(db, "users.bloomdb");
```

Saves complete filter state to binary file.

#### 5. Load from Disk

```c
BloomDB* loaded = bloomdb_load("users.bloomdb");
```

Restores filter with all previously inserted elements.

#### 6. Cleanup

```c
bloomdb_free(db);
```

Releases all allocated memory.

### Use Cases

**Phase 1 is suitable for:**
- Cache invalidation (checking if item might be cached)
- Preventing expensive lookups (database, network)
- Deduplication (checking if item was seen before)
- Spell checking (dictionary membership)
- Network packet filtering
- Weak password detection

**Limitations:**
- Single-threaded only
- Cannot delete elements
- In-memory only (must load entire filter)
- False positive rate increases as filter fills

## Running Tests

### Unit Tests (Correctness)

```bash
# In Docker
docker exec bloomdb-dev make test

# Native
make test
```

Tests verify:
- Bit array operations
- Hash function properties
- Bloom filter insert/query
- File persistence

### Memory Tests (Valgrind)

```bash
# In Docker
docker exec bloomdb-dev make valgrind

# Native
make valgrind
```

Detects:
- Memory leaks
- Invalid reads/writes
- Use of uninitialized memory

### Memory Tests (Address Sanitizer)

```bash
# In Docker
docker exec bloomdb-dev make asan

# Native
make asan
```

Detects:
- Buffer overflows
- Use-after-free
- Double-free
- Memory corruption

### All Tests

```bash
# In Docker
docker exec bloomdb-dev bash tests/run_tests.sh

# Native
bash tests/run_tests.sh
```

Runs all three test suites with summary output.

## Running Benchmarks

### Performance Measurement

```bash
# In Docker
docker exec bloomdb-dev make benchmark

# Native
make benchmark
```

Output includes:
- Average latency per operation
- P50, P90, P99 percentiles
- JSON export for analysis

Example results:
```
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
```

### View Results

```bash
cat benchmark_results.json
```

## Building from Source

### Standard Build

```bash
make build
```

Produces optimized executable: `bloomdb`

### Debug Build

```bash
make debug
```

Produces debug executable with symbols: `bloomdb_dbg`

### All Targets

```bash
make clean      # Remove all build artifacts
make build      # Build main executable
make test       # Build and run unit tests
make valgrind   # Run Valgrind memory tests
make asan       # Run ASan memory tests
make benchmark  # Run performance benchmarks
make test-all   # Run all tests (takes several minutes)
```

## Project Structure

```
bloomdb/
├── include/           # Header files
│   ├── bloomdb.h      # Main API
│   ├── bitarray.h     # Bit operations
│   ├── hash64.h       # Hash function
│   └── storage.h      # Persistence
├── src/               # Implementation
│   ├── bloomdb.c
│   ├── bitarray.c
│   ├── hash64.c
│   ├── storage.c
│   └── main.c         # Example usage
├── tests/             # Test suite
│   ├── test_*.c       # Unit tests
│   ├── benchmark_pro.c # Performance tests
│   └── run_tests.sh   # Test runner
├── docs/              # Documentation
│   └── phase1/        # Phase 1 docs
├── Makefile           # Build system
├── Dockerfile         # Docker image
└── docker-compose.yml # Docker setup
```

## Troubleshooting

### Docker container won't start

```bash
docker-compose down
docker-compose up -d --build
```

### Compilation errors

Verify GCC version:
```bash
gcc --version  # Should be 4.8 or higher
```

### Tests fail

Clean and rebuild:
```bash
make clean
make test
```

### Permission denied

Make sure executables are not running:
```bash
pkill bloomdb
make clean
make build
```

### Valgrind not found

Install Valgrind:
```bash
# Ubuntu/Debian
sudo apt-get install valgrind

# macOS
brew install valgrind
```

## Next Steps

After successful installation:

1. **Read the documentation**: See `docs/phase1/` for detailed information
2. **Explore the code**: Start with `src/main.c` for usage examples
3. **Run benchmarks**: Measure performance on your hardware
4. **Experiment**: Try different filter sizes and hash function counts
5. **Integrate**: Use as a library in your own projects

## Getting Help

- **Documentation**: See `docs/phase1/README.md`
- **Issues**: Report bugs on GitHub Issues
- **Source**: Review code in `src/` directory

## Version Information

This installation guide is for BloomDB Phase 1 (v1.0.0), which includes:
- Core Bloom filter implementation
- Binary persistence
- Comprehensive test suite
- Professional benchmarking framework
