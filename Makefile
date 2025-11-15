CC=gcc
CFLAGS=-Wall -Wextra -O2 -g -Iinclude
ASAN_FLAGS=-fsanitize=address -g -O0 -Iinclude
VALGRIND=valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1

SRC=src/bloomdb.c src/bitarray.c src/hash64.c src/storage.c
MAIN=src/main.c

# Test executables
TEST_BITARRAY=tests/test_bitarray
TEST_HASH64=tests/test_hash64
TEST_BLOOMDB=tests/test_bloomdb
TEST_STORAGE=tests/test_storage
TEST_BLOOMDB_EX=tests/test_bloomdb_ex
TEST_STORAGE_EX=tests/test_storage_ex
TEST_HELPERS=tests/test_helpers

# ASan test executables
TEST_BITARRAY_ASAN=tests/test_bitarray_asan
TEST_HASH64_ASAN=tests/test_hash64_asan
TEST_BLOOMDB_ASAN=tests/test_bloomdb_asan
TEST_STORAGE_ASAN=tests/test_storage_asan
TEST_BLOOMDB_EX_ASAN=tests/test_bloomdb_ex_asan
TEST_STORAGE_EX_ASAN=tests/test_storage_ex_asan
TEST_HELPERS_ASAN=tests/test_helpers_asan

all: build

build:
	$(CC) $(CFLAGS) $(SRC) $(MAIN) -o bloomdb

build-asan-main:
	$(CC) $(CFLAGS) -fsanitize=address $(SRC) $(MAIN) -o bloomdb_asan

val:
	$(CC) $(CFLAGS) $(SRC) $(MAIN) -o bloomdb_val

debug:
	$(CC) $(CFLAGS) -g $(SRC) $(MAIN) -o bloomdb_dbg

# Build tests
build-tests: $(TEST_BITARRAY) $(TEST_HASH64) $(TEST_BLOOMDB) $(TEST_STORAGE) $(TEST_BLOOMDB_EX) $(TEST_STORAGE_EX) $(TEST_HELPERS)

$(TEST_BITARRAY): tests/test_bitarray.c src/bitarray.c
	$(CC) $(CFLAGS) src/bitarray.c tests/test_bitarray.c -o $(TEST_BITARRAY)

$(TEST_HASH64): tests/test_hash64.c src/hash64.c
	$(CC) $(CFLAGS) src/hash64.c tests/test_hash64.c -o $(TEST_HASH64)

$(TEST_BLOOMDB): tests/test_bloomdb.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(CFLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_bloomdb.c -o $(TEST_BLOOMDB)

$(TEST_STORAGE): tests/test_storage.c $(SRC)
	$(CC) $(CFLAGS) $(SRC) tests/test_storage.c -o $(TEST_STORAGE)

$(TEST_BLOOMDB_EX): tests/test_bloomdb_ex.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(CFLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_bloomdb_ex.c -o $(TEST_BLOOMDB_EX)

$(TEST_STORAGE_EX): tests/test_storage_ex.c $(SRC)
	$(CC) $(CFLAGS) $(SRC) tests/test_storage_ex.c -o $(TEST_STORAGE_EX)

$(TEST_HELPERS): tests/test_helpers.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(CFLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_helpers.c -o $(TEST_HELPERS)

# Build ASan tests
build-asan: $(TEST_BITARRAY_ASAN) $(TEST_HASH64_ASAN) $(TEST_BLOOMDB_ASAN) $(TEST_STORAGE_ASAN) $(TEST_BLOOMDB_EX_ASAN) $(TEST_STORAGE_EX_ASAN) $(TEST_HELPERS_ASAN)

$(TEST_BITARRAY_ASAN): tests/test_bitarray.c src/bitarray.c
	$(CC) $(ASAN_FLAGS) src/bitarray.c tests/test_bitarray.c -o $(TEST_BITARRAY_ASAN)

$(TEST_HASH64_ASAN): tests/test_hash64.c src/hash64.c
	$(CC) $(ASAN_FLAGS) src/hash64.c tests/test_hash64.c -o $(TEST_HASH64_ASAN)

$(TEST_BLOOMDB_ASAN): tests/test_bloomdb.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(ASAN_FLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_bloomdb.c -o $(TEST_BLOOMDB_ASAN)

$(TEST_STORAGE_ASAN): tests/test_storage.c $(SRC)
	$(CC) $(ASAN_FLAGS) $(SRC) tests/test_storage.c -o $(TEST_STORAGE_ASAN)

$(TEST_BLOOMDB_EX_ASAN): tests/test_bloomdb_ex.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(ASAN_FLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_bloomdb_ex.c -o $(TEST_BLOOMDB_EX_ASAN)

$(TEST_STORAGE_EX_ASAN): tests/test_storage_ex.c $(SRC)
	$(CC) $(ASAN_FLAGS) $(SRC) tests/test_storage_ex.c -o $(TEST_STORAGE_EX_ASAN)

$(TEST_HELPERS_ASAN): tests/test_helpers.c src/bloomdb.c src/bitarray.c src/hash64.c
	$(CC) $(ASAN_FLAGS) src/bitarray.c src/hash64.c src/bloomdb.c tests/test_helpers.c -o $(TEST_HELPERS_ASAN)

# Run all tests
test: build-tests
	@echo "Running tests..."
	@./$(TEST_BITARRAY)
	@./$(TEST_HASH64)
	@./$(TEST_BLOOMDB)
	@./$(TEST_STORAGE)
	@./$(TEST_BLOOMDB_EX)
	@./$(TEST_STORAGE_EX)
	@./$(TEST_HELPERS)
	@echo "All tests passed! ✅"

# Run Valgrind memory tests
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
	@echo "→ test_bloomdb_ex"
	@$(VALGRIND) ./$(TEST_BLOOMDB_EX)
	@echo "→ test_storage_ex"
	@$(VALGRIND) ./$(TEST_STORAGE_EX)
	@echo "→ test_helpers"
	@$(VALGRIND) ./$(TEST_HELPERS)
	@echo "All Valgrind tests passed! 🧪"

# Run ASan tests
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
	@echo "→ test_bloomdb_ex_asan"
	@./$(TEST_BLOOMDB_EX_ASAN)
	@echo "→ test_storage_ex_asan"
	@./$(TEST_STORAGE_EX_ASAN)
	@echo "→ test_helpers_asan"
	@./$(TEST_HELPERS_ASAN)
	@echo "All ASan tests passed! 💥"

# Run ALL tests (normal + valgrind + asan)
test-all: test valgrind asan
	@echo "═══════════════════════════════════════"
	@echo "🎉 ALL TESTS PASSED (normal + valgrind + asan)"
	@echo "═══════════════════════════════════════"

# Run test script
run-tests:
	@bash tests/run_tests.sh

# Benchmark PRO
benchmark: tests/benchmark_pro
	@echo "🔥 Running BloomDB PRO Benchmark Suite..."
	@./tests/benchmark_pro

tests/benchmark_pro: tests/benchmark_pro.c $(SRC)
	$(CC) -O3 -march=native -Iinclude $(SRC) tests/benchmark_pro.c -o tests/benchmark_pro -lm

clean:
	rm -f bloomdb bloomdb_asan bloomdb_val bloomdb_dbg
	rm -f $(TEST_BITARRAY) $(TEST_HASH64) $(TEST_BLOOMDB) $(TEST_STORAGE) $(TEST_BLOOMDB_EX) $(TEST_STORAGE_EX) $(TEST_HELPERS)
	rm -f $(TEST_BITARRAY_ASAN) $(TEST_HASH64_ASAN) $(TEST_BLOOMDB_ASAN) $(TEST_STORAGE_ASAN) $(TEST_BLOOMDB_EX_ASAN) $(TEST_STORAGE_EX_ASAN) $(TEST_HELPERS_ASAN)
	rm -f tests/benchmark_pro
	rm -f test_filter.bloomdb benchmark_results.json test_ex.bloom test_corrupt.bloom test_truncated.bloom
