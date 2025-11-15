# Core Modules

## bitarray

### Purpose
Provides efficient bit-level manipulation operations for the underlying Bloom filter storage.

### Interface

```c
void bitarray_set(uint8_t* arr, size_t bit);
bool bitarray_get(const uint8_t* arr, size_t bit);
```

### Implementation Details

**bitarray_set**
- Locates the target byte: `arr[bit >> 3]`
- Computes bit position: `bit & 7`
- Sets the bit using OR operation: `|= (1 << position)`
- Time complexity: O(1)
- Side effects: Modifies the bit array in place

**bitarray_get**
- Locates the target byte: `arr[bit >> 3]`
- Computes bit position: `bit & 7`
- Tests the bit using AND operation: `& (1 << position)`
- Returns true if bit is set, false otherwise
- Time complexity: O(1)
- Side effects: None (read-only)

### Performance Characteristics
- No branching in critical path
- Single memory access per operation
- Cache-friendly for sequential access patterns
- Benchmark: ~0.40 ns/op average

### Usage Example
```c
uint8_t bits[128] = {0};  // 1024 bits
bitarray_set(bits, 42);
bool is_set = bitarray_get(bits, 42);  // true
```

---

## hash64

### Purpose
Computes 64-bit hash values for arbitrary byte sequences. Used to derive bit indices for Bloom filter operations.

### Interface

```c
uint64_t hash64(const void* key, size_t len, uint64_t seed);
```

### Implementation Details

The hash function implements a simple but effective mixing algorithm:
1. Initialize hash with seed value
2. For each byte in input:
   - XOR byte into hash
   - Multiply by prime constant (0x5bd1e995)
   - XOR hash with itself right-shifted by 15 bits
3. Return final hash value

### Properties
- Deterministic: Same input + seed produces same output
- Avalanche effect: Small input changes produce large hash changes
- Uniform distribution: Output values spread across 64-bit range
- Fast: Benchmark shows ~11.74 ns/op average

### Limitations
- Not cryptographically secure
- Designed for speed, not collision resistance against adversarial inputs
- Suitable for Bloom filters and hash tables

### Double Hashing Strategy
BloomDB uses this function twice per operation:
```c
h1 = hash64(key, len, seed)
h2 = hash64(key, len, seed + i + 1)
bit_index = (h1 + i * h2) % bit_count
```

This eliminates the need for k independent hash functions.

### Future Improvements
Planned replacement with xxHash or MurmurHash3 for:
- Better distribution characteristics
- Faster performance
- Industry-standard implementation

---

## bloomdb

### Purpose
Core Bloom filter implementation. Orchestrates hashing and bit array operations to provide probabilistic set membership testing.

### Interface

```c
BloomDB* bloomdb_create(size_t bits, int num_hashes, uint64_t seed);
void bloomdb_free(BloomDB* db);
bool bloomdb_insert(BloomDB* db, const void* key, size_t len);
bool bloomdb_might_contain(const BloomDB* db, const void* key, size_t len);
```

### Data Structure

```c
typedef struct {
    uint8_t* bitarray;   // Bit storage
    size_t bit_count;    // Total bits (m)
    size_t byte_count;   // Array size in bytes
    int num_hashes;      // Hash functions count (k)
    uint64_t seed;       // Reproducibility seed
} BloomDB;
```

### Operations

**bloomdb_create**
- Allocates BloomDB structure
- Allocates and zero-initializes bit array
- Calculates byte count: `(bits + 7) / 8`
- Returns NULL on allocation failure

**bloomdb_free**
- Frees bit array memory
- Frees BloomDB structure
- Safe to call with NULL pointer

**bloomdb_insert**
- Computes k hash values using double hashing
- Sets corresponding bits in the array
- Always succeeds for valid input
- Time complexity: O(k)

**bloomdb_might_contain**
- Computes k hash values using double hashing
- Checks if all corresponding bits are set
- Returns false immediately if any bit is unset (early exit)
- Returns true if all bits are set (possible false positive)
- Time complexity: O(k) worst case, faster on average

### Internal Functions

**get_bit_index**
```c
static size_t get_bit_index(const BloomDB* db, const void* key, size_t len, int hash_num)
```
Implements double hashing:
- h1 = hash64(key, len, seed)
- h2 = hash64(key, len, seed + hash_num + 1)
- return (h1 + hash_num * h2) % bit_count

**set_bit / get_bit**
Inline wrappers around bitarray operations for internal use.

### False Positive Rate

The false positive probability is approximately:
```
p ≈ (1 - e^(-kn/m))^k
```
Where:
- k = num_hashes
- n = number of inserted elements
- m = bit_count

Optimal k for given m and n:
```
k = (m/n) * ln(2)
```

### Performance
- Insert: ~193.56 ns/op average
- Query: Similar to insert, faster with early exit
- Memory: m bits + metadata overhead (32 bytes)

---

## storage

### Purpose
Handles serialization and deserialization of BloomDB structures to/from binary files.

### Interface

```c
bool bloomdb_save(const BloomDB* db, const char* path);
BloomDB* bloomdb_load(const char* path);
```

### File Format Specification

```
Offset  Size    Field
0       8       bit_count (size_t)
8       8       byte_count (size_t)
16      4       num_hashes (int)
20      8       seed (uint64_t)
28      N       bitarray (N = byte_count)
```

Total file size: 28 + byte_count bytes

### Operations

**bloomdb_save**
1. Opens file for binary writing
2. Writes metadata fields sequentially using fwrite
3. Writes entire bit array in single operation
4. Closes file
5. Returns true on success, false on any failure

Error conditions:
- NULL pointer inputs
- File open failure
- Write errors (disk full, permissions, etc.)

**bloomdb_load**
1. Opens file for binary reading
2. Reads metadata fields
3. Creates new BloomDB with loaded parameters
4. Reads bit array directly into allocated memory
5. Returns BloomDB pointer or NULL on failure

Error conditions:
- NULL path
- File not found or unreadable
- Incomplete reads (corrupted file)
- Memory allocation failure

### Portability Considerations

Current implementation assumes:
- Matching endianness between writer and reader
- Same size_t width (32-bit vs 64-bit)
- Compatible int representation

Future versions may add:
- Endianness markers
- Version headers
- Checksum validation
- Platform-independent size encoding

### Performance
- Write: Limited by disk I/O speed
- Read: Single allocation + bulk read (very fast)
- No parsing overhead
- Suitable for memory-mapped loading (future optimization)

### Usage Pattern

```c
// Save
BloomDB* db = bloomdb_create(1000000, 5, 42);
// ... insert data ...
bloomdb_save(db, "filter.bloomdb");
bloomdb_free(db);

// Load
BloomDB* loaded = bloomdb_load("filter.bloomdb");
bool exists = bloomdb_might_contain(loaded, key, len);
bloomdb_free(loaded);
```

### File Extension Convention
`.bloomdb` is used by convention but not enforced. Any path can be used.
