# Testing Infrastructure

## Overview

Phase 1 includes a comprehensive testing suite covering correctness, memory safety, and performance. The testing infrastructure is designed to catch bugs early and provide confidence in the implementation.

## Test Organization

```
tests/
├── test_bitarray.c      # Bit manipulation correctness
├── test_hash64.c        # Hash function properties
├── test_bloomdb.c       # Bloom filter operations
├── test_storage.c       # Persistence functionality
├── benchmark_pro.c      # Performance measurements
└── run_tests.sh         # Automated test runner
```

## Unit Tests

### test_bitarray.c

**Purpose**: Verify bit array set and get operations

**Test Cases**:
- Initial state verification (all bits zero)
- Individual bit setting at various positions
- Bit retrieval after setting
- Boundary testing (first byte, second byte, etc.)
- Idempotency (setting same bit twice)
- Non-interference (other bits remain unchanged)

**Coverage**:
- Edge cases: bit 0, bit 7, bit 8, bit 15
- Both bytes in a 2-byte array
- Multiple operations on same array

**Assertions**: 16 total

### test_hash64.c

**Purpose**: Validate hash function properties

**Test Cases**:

1. **Determinism Test**
   - Same input and seed produce identical output
   - Multiple invocations return consistent results

2. **Key Differentiation Test**
   - Different keys produce different hashes
   - Tests single-character and multi-character keys
   - Verifies avalanche effect

3. **Seed Sensitivity Test**
   - Different seeds produce different hashes
   - Same key with different seeds diverge
   - Tests seed values: 1, 2, 9999

**Expected Behavior**:
- Zero hash collisions in test set (probabilistic but highly likely)
- Deterministic output for given input+seed pair

### test_bloomdb.c

**Purpose**: Test complete Bloom filter functionality

**Setup**:
- Large filter: 100,000 bits
- 5 hash functions
- Seed: 42

**Test Cases**:

1. **Creation Test**
   - Successful allocation
   - Non-NULL return value

2. **Insertion Test**
   - Insert 5 email addresses
   - All insertions return true
   - Verify all inserted items are found

3. **Query Test**
   - Check 5 non-inserted items
   - Expect at least one to return false
   - Tests false positive behavior

4. **Memory Cleanup**
   - Successful deallocation

**Coverage**:
- Basic operations (create, insert, query, free)
- False negative impossibility (inserted items always found)
- False positive awareness (not all queries return true)

### test_storage.c

**Purpose**: Verify persistence and recovery

**Test File**: `test_filter.bloomdb`

**Test Cases**:

1. **Save Operation**
   - Create filter with known parameters
   - Insert test data
   - Save to disk
   - Verify save success

2. **Load Operation**
   - Load from disk
   - Verify non-NULL return

3. **Metadata Verification**
   - bit_count matches
   - byte_count matches
   - num_hashes matches
   - seed matches
   - bitarray pointer is valid

4. **Data Integrity**
   - Previously inserted items still found
   - Query results identical to pre-save state

5. **Cleanup**
   - Remove temporary test file

**Coverage**:
- Full round-trip (save → load)
- Metadata preservation
- Data integrity across persistence

## Memory Safety Tests

### Valgrind Integration

**Command**:
```bash
valgrind --leak-check=full --show-leak-kinds=all \
         --track-origins=yes --error-exitcode=1 \
         ./tests/test_*
```

**Checks**:
- Heap memory leaks
- Invalid memory reads
- Invalid memory writes
- Use of uninitialized values
- Double frees
- Mismatched allocation/deallocation

**Expected Output**:
```
HEAP SUMMARY:
  in use at exit: 0 bytes in 0 blocks
  total heap usage: N allocs, N frees

All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors from 0 contexts
```

### Address Sanitizer (ASan)

**Compilation**:
```bash
gcc -fsanitize=address -g -O0 -Iinclude src/*.c tests/test_*.c
```

**Detects**:
- Buffer overflows (heap and stack)
- Use-after-free
- Use-after-return
- Use-after-scope
- Double-free
- Memory leaks

**Advantages over Valgrind**:
- Faster execution (2-3x slowdown vs 10-50x)
- Better error messages
- Detects more bug types
- Compile-time instrumentation

**Trade-off**:
- Requires recompilation
- Increases binary size
- Uses more memory

## Test Execution

### Manual Execution

```bash
# Individual tests
make build-tests
./tests/test_bitarray
./tests/test_hash64
./tests/test_bloomdb
./tests/test_storage

# Memory tests
make valgrind

# ASan tests
make asan
```

### Automated Test Runner

**Script**: `tests/run_tests.sh`

**Features**:
- Sequential execution of all test suites
- Color-coded output (requires terminal support)
- Proper exit codes for CI/CD integration
- Summary report
- Automatic cleanup

**Test Suites**:
1. Logical tests (correctness)
2. Valgrind tests (memory leaks)
3. ASan tests (memory corruption)

**Output Format**:
```
Running: Logical Tests
== test_bitarray ==
✓ test_bitarray: OK
...
✓ Logical Tests: PASSED

Running: Valgrind Memory Tests
...
All heap blocks were freed -- no leaks are possible
✓ Valgrind Memory Tests: PASSED

Running: ASan Tests
...
✓ ASan Tests: PASSED

ALL TESTS PASSED
```

## Test Results Interpretation

### Success Criteria

All tests must:
- Complete without assertion failures
- Return exit code 0
- Show "OK" or "PASSED" status
- Report zero memory leaks
- Report zero memory errors

### Failure Modes

**Assertion Failure**:
- Indicates logic bug
- Shows line number and condition
- Requires code fix

**Memory Leak**:
- Indicates missing free()
- Valgrind shows allocation site
- Requires cleanup fix

**Memory Corruption**:
- ASan shows exact location and type
- Often indicates buffer overflow or use-after-free
- Requires bounds checking or lifetime fix

## Continuous Integration

The test suite is designed for CI/CD integration:
- Fast execution (under 30 seconds total)
- Clear pass/fail indication
- Non-interactive (no user input required)
- Proper exit codes (0 = success, non-zero = failure)
- Can run in Docker container

**Typical CI Pipeline**:
```yaml
- docker-compose up -d
- docker exec bloomdb-dev make test
- docker exec bloomdb-dev make valgrind
- docker exec bloomdb-dev make asan
```

## Test Coverage

**Current Coverage**:
- All public API functions tested
- Critical internal functions covered
- Edge cases included
- Error paths verified

**Not Covered** (Future Work):
- Concurrent access scenarios
- Extremely large filters (memory limits)
- Corrupted file handling
- Platform-specific behavior

## Adding New Tests

**Template**:
```c
#include <stdio.h>
#include <assert.h>
#include "module.h"

int main(void) {
    printf("== test_name ==\n");
    
    // Setup
    // ...
    
    // Test cases
    assert(condition);
    
    // Cleanup
    // ...
    
    printf("✓ test_name: OK\n");
    return 0;
}
```

**Integration**:
1. Add source file to `tests/`
2. Add Makefile target
3. Update `run_tests.sh` if needed
4. Document in this file
