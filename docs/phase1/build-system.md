# Build System

## Overview

The build system uses GNU Make to manage compilation, testing, and benchmarking. It provides multiple targets for different workflows and configurations.

## Makefile Structure

### Variables

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -Iinclude
ASAN_FLAGS = -fsanitize=address -g -O0 -Iinclude
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1
```

**CC**: Compiler to use (gcc by default)
- Can be overridden: `make CC=clang`
- Compatible with gcc and clang

**CFLAGS**: Standard compilation flags
- `-Wall -Wextra`: Enable all warnings
- `-O2`: Optimization level 2 (good balance)
- `-g`: Include debug symbols
- `-Iinclude`: Add include directory to search path

**ASAN_FLAGS**: Address Sanitizer compilation
- `-fsanitize=address`: Enable ASan instrumentation
- `-g`: Debug symbols for error reports
- `-O0`: Disable optimization for clearer errors
- `-Iinclude`: Include directory

**VALGRIND**: Valgrind command template
- `--leak-check=full`: Detailed leak information
- `--show-leak-kinds=all`: Show all leak types
- `--track-origins=yes`: Track uninitialized values
- `--error-exitcode=1`: Non-zero exit on errors

### Source Files

```makefile
SRC = src/bloomdb.c src/bitarray.c src/hash64.c src/storage.c
MAIN = src/main.c
```

**SRC**: Core library sources
- Used by all builds
- Order doesn't matter (no circular dependencies)

**MAIN**: Application entry point
- Separate from library for testing
- Can be replaced with other main files

### Test Executables

```makefile
TEST_BITARRAY = tests/test_bitarray
TEST_HASH64 = tests/test_hash64
TEST_BLOOMDB = tests/test_bloomdb
TEST_STORAGE = tests/test_storage
```

Variables for test binary names, used in multiple targets.

### ASan Test Executables

```makefile
TEST_BITARRAY_ASAN = tests/test_bitarray_asan
TEST_HASH64_ASAN = tests/test_hash64_asan
TEST_BLOOMDB_ASAN = tests/test_bloomdb_asan
TEST_STORAGE_ASAN = tests/test_storage_asan
```

Separate binaries for ASan-instrumented tests.

## Build Targets

### all (default)

```makefile
all: build
```

Default target. Runs `build` target.

**Usage**: `make` or `make all`

### build

```makefile
build:
	$(CC) $(CFLAGS) $(SRC) $(MAIN) -o bloomdb
```

Compiles main application.

**Output**: `bloomdb` executable
**Usage**: `make build`
**Purpose**: Standard development build with optimizations

### build-asan-main

```makefile
build-asan-main:
	$(CC) $(CFLAGS) -fsanitize=address $(SRC) $(MAIN) -o bloomdb_asan
```

Compiles main application with Address Sanitizer.

**Output**: `bloomdb_asan` executable
**Usage**: `make build-asan-main`
**Purpose**: Memory debugging of main application

### val

```makefile
val:
	$(CC) $(CFLAGS) $(SRC) $(MAIN) -o bloomdb_val
```

Compiles optimized build for Valgrind testing.

**Output**: `bloomdb_val` executable
**Usage**: `make val`
**Note**: Same as build but with explicit target name

### debug

```makefile
debug:
	$(CC) $(CFLAGS) -g $(SRC) $(MAIN) -o bloomdb_dbg
```

Compiles with maximum debug information.

**Output**: `bloomdb_dbg` executable
**Usage**: `make debug`
**Purpose**: For use with gdb or other debuggers

## Test Targets

### build-tests

```makefile
build-tests: $(TEST_BITARRAY) $(TEST_HASH64) $(TEST_BLOOMDB) $(TEST_STORAGE)
```

Builds all unit test executables.

**Dependencies**: Individual test targets
**Usage**: `make build-tests`
**Output**: Four test executables in `tests/`

### Individual Test Targets

```makefile
$(TEST_BITARRAY): tests/test_bitarray.c src/bitarray.c
	$(CC) $(CFLAGS) src/bitarray.c tests/test_bitarray.c -o $(TEST_BITARRAY)

$(TEST_HASH64): tests/test_hash64.c src/hash64.c
	$(CC) $(CFLAGS) src/hash64.c tests/test_hash64.c -o $(TEST_HASH64)

$(TEST_BLOOMDB): tests/test_bloomdb.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(CFLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_bloomdb.c -o $(TEST_BLOOMDB)

$(TEST_STORAGE): tests/test_storage.c $(SRC)
	$(CC) $(CFLAGS) $(SRC) tests/test_storage.c -o $(TEST_STORAGE)
```

Each test links only necessary sources:
- test_bitarray: bitarray.c only
- test_hash64: hash64.c only
- test_bloomdb: bloomdb.c + dependencies
- test_storage: all sources

**Benefit**: Faster compilation, clearer dependencies

### test

```makefile
test: build-tests
	@echo "Running tests..."
	@./$(TEST_BITARRAY)
	@./$(TEST_HASH64)
	@./$(TEST_BLOOMDB)
	@./$(TEST_STORAGE)
	@echo "All tests passed! ✅"
```

Builds and runs all unit tests.

**Usage**: `make test`
**Output**: Test results to stdout
**Exit Code**: 0 on success, non-zero on failure

### build-asan

```makefile
build-asan: $(TEST_BITARRAY_ASAN) $(TEST_HASH64_ASAN) $(TEST_BLOOMDB_ASAN) $(TEST_STORAGE_ASAN)
```

Builds ASan-instrumented test executables.

**Usage**: `make build-asan`
**Output**: Four `*_asan` executables

### asan

```makefile
asan: build-asan
	@echo "Running ASan (Address Sanitizer) tests..."
	@echo "→ test_bitarray_asan"
	@./$(TEST_BITARRAY_ASAN)
	@echo "→ test_hash64_asan"
	@./$(TEST_HASH64_ASAN)
	@echo "→ test_bloomdb_asan"
	@./$(TEST_BLOOMDB_ASAN)
	@echo "→ test_storage_asan"
	@./$(TEST_STORAGE_ASAN)
	@echo "All ASan tests passed! 💥"
```

Builds and runs ASan tests.

**Usage**: `make asan`
**Output**: ASan error reports (if any)
**Exit Code**: 0 if no memory errors detected

### valgrind

```makefile
valgrind: build-tests
	@echo "Running Valgrind memory tests..."
	@echo "→ test_bitarray"
	@$(VALGRIND) ./$(TEST_BITARRAY)
	@echo "→ test_hash64"
	@$(VALGRIND) ./$(TEST_HASH64)
	@echo "→ test_bloomdb"
	@$(VALGRIND) ./$(TEST_BLOOMDB)
	@echo "→ test_storage"
	@$(VALGRIND) ./$(TEST_STORAGE)
	@echo "All Valgrind tests passed! 🧪"
```

Runs tests under Valgrind memory checker.

**Usage**: `make valgrind`
**Duration**: Slower than regular tests (10-50x)
**Output**: Valgrind leak reports
**Exit Code**: Non-zero if leaks detected

### test-all

```makefile
test-all: test valgrind asan
	@echo "════════════════════════════════════════"
	@echo "🎉 ALL TESTS PASSED (normal + valgrind + asan)"
	@echo "════════════════════════════════════════"
```

Runs complete test suite: unit tests, Valgrind, and ASan.

**Usage**: `make test-all`
**Duration**: Several minutes
**Purpose**: Pre-commit verification

### run-tests

```makefile
run-tests:
	@bash tests/run_tests.sh
```

Executes automated test runner script.

**Usage**: `make run-tests`
**Equivalent to**: `test-all` but with prettier output

## Benchmark Targets

### benchmark

```makefile
benchmark: tests/benchmark_pro
	@echo "🔥 Running BloomDB PRO Benchmark Suite..."
	@./tests/benchmark_pro

tests/benchmark_pro: tests/benchmark_pro.c $(SRC)
	$(CC) -O3 -march=native -Iinclude $(SRC) tests/benchmark_pro.c -o tests/benchmark_pro -lm
```

Compiles and runs performance benchmarks.

**Compilation**:
- `-O3`: Aggressive optimization
- `-march=native`: CPU-specific instructions
- `-lm`: Math library (for sorting)

**Usage**: `make benchmark`
**Output**: 
- Performance statistics to stdout
- `benchmark_results.json` file

## Cleanup Target

### clean

```makefile
clean:
	rm -f bloomdb bloomdb_asan bloomdb_val bloomdb_dbg
	rm -f $(TEST_BITARRAY) $(TEST_HASH64) $(TEST_BLOOMDB) $(TEST_STORAGE)
	rm -f $(TEST_BITARRAY_ASAN) $(TEST_HASH64_ASAN) $(TEST_BLOOMDB_ASAN) $(TEST_STORAGE_ASAN)
	rm -f tests/benchmark_pro
	rm -f test_filter.bloomdb benchmark_results.json
```

Removes all build artifacts and temporary files.

**Usage**: `make clean`
**Removes**:
- Main executables (all variants)
- Test executables (normal and ASan)
- Benchmark executable
- Temporary test files
- Benchmark results

## Common Workflows

### Development Cycle

```bash
# Make changes to code
vim src/bloomdb.c

# Quick compile check
make build

# Run tests
make test

# If tests pass, run full suite
make test-all
```

### Performance Testing

```bash
# Clean build
make clean
make benchmark > baseline.txt

# Make optimization
vim src/hash64.c

# Re-benchmark
make benchmark > optimized.txt

# Compare
diff baseline.txt optimized.txt
```

### Memory Debugging

```bash
# For quick checks
make asan

# For detailed analysis
make valgrind

# For both
make test-all
```

### CI/CD Pipeline

```bash
# Full verification
make clean
make test-all
make benchmark
```

## Customization

### Changing Compiler

```bash
make CC=clang
```

### Adding Flags

```bash
make CFLAGS="-Wall -Wextra -O3 -march=native -Iinclude"
```

### Debug Build

```bash
make CFLAGS="-Wall -Wextra -O0 -g -Iinclude"
```

### Cross-Compilation

```bash
make CC=arm-linux-gnueabihf-gcc
```

## Dependencies

### Required

- gcc or clang
- GNU Make
- POSIX environment (Linux, macOS, WSL)

### Optional

- Valgrind (for memory testing)
- Address Sanitizer support in compiler (gcc 4.8+, clang 3.1+)

### Docker Environment

The provided Docker container includes all dependencies:

```bash
docker exec bloomdb-dev make test-all
docker exec bloomdb-dev make benchmark
```

## Troubleshooting

### "No rule to make target"

Check that all source files exist in expected locations.

### Compilation Errors

Verify compiler version supports required features:
```bash
gcc --version  # Should be 4.8+
```

### Permission Denied

Ensure executables are not running:
```bash
make clean
make build
```

### Valgrind Not Found

Install Valgrind:
```bash
# Ubuntu/Debian
apt-get install valgrind

# macOS
brew install valgrind
```

### ASan Not Working

Ensure compiler supports it:
```bash
gcc -fsanitize=address --version
```

## Future Enhancements

Planned improvements:
- Parallel compilation with `-j`
- Separate library target (libbloomdb.a)
- Install target for system-wide installation
- Package generation (deb, rpm)
- CMake alternative for cross-platform builds
