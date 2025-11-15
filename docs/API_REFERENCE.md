# BloomDB API Reference

## Error Handling

### BloomDBError

Error codes returned by `*_ex` functions:

```c
typedef enum {
    BLOOMDB_OK = 0,                // Success
    BLOOMDB_ERR_INVALID_ARGUMENT,  // Invalid function argument
    BLOOMDB_ERR_ALLOC,             // Memory allocation failed
    BLOOMDB_ERR_FILE_IO,           // File I/O error
    BLOOMDB_ERR_FORMAT             // Invalid file format
} BloomDBError;
```

### bloomdb_strerror

```c
const char* bloomdb_strerror(BloomDBError err);
```

Returns a human-readable string describing the error code.

**Parameters:**
- `err`: Error code to convert to string

**Returns:** Constant string describing the error

**Example:**
```c
BloomDBError err = bloomdb_create_ex(0, 3, 42, &db);
if (err != BLOOMDB_OK) {
    printf("Error: %s\n", bloomdb_strerror(err));
}
```

---

## Core API

### BloomDB Structure

```c
typedef struct BloomDB {
    uint8_t* bitarray;
    size_t bit_count;
    int num_hashes;
    uint64_t seed;
} BloomDB;
```

Internal structure representing a Bloom filter. **Do not access fields directly.**

---

## Creation and Destruction

### bloomdb_create

```c
BloomDB* bloomdb_create(size_t bits, int num_hashes, uint64_t seed);
```

Creates a new Bloom filter (simple API).

**Parameters:**
- `bits`: Number of bits in the filter (must be > 0)
- `num_hashes`: Number of hash functions to use (must be > 0)
- `seed`: Random seed for hash functions

**Returns:** Pointer to BloomDB on success, NULL on failure

**Example:**
```c
BloomDB* db = bloomdb_create(10000, 5, 42);
if (!db) {
    fprintf(stderr, "Failed to create BloomDB\n");
}
```

### bloomdb_create_ex

```c
BloomDBError bloomdb_create_ex(size_t bits, int num_hashes, uint64_t seed, BloomDB** out_db);
```

Creates a new Bloom filter with explicit error handling.

**Parameters:**
- `bits`: Number of bits in the filter (must be > 0)
- `num_hashes`: Number of hash functions to use (must be > 0)
- `seed`: Random seed for hash functions
- `out_db`: Output parameter for the created BloomDB (must not be NULL)

**Returns:**
- `BLOOMDB_OK` on success
- `BLOOMDB_ERR_INVALID_ARGUMENT` if parameters are invalid
- `BLOOMDB_ERR_ALLOC` if memory allocation fails

**Example:**
```c
BloomDB* db = NULL;
BloomDBError err = bloomdb_create_ex(10000, 5, 42, &db);
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Error: %s\n", bloomdb_strerror(err));
    return 1;
}
```

### bloomdb_free

```c
void bloomdb_free(BloomDB* db);
```

Frees a Bloom filter and all associated memory. Safe to call with NULL.

**Parameters:**
- `db`: Bloom filter to free (can be NULL)

**Example:**
```c
bloomdb_free(db);
```

---

## Insertion

### bloomdb_insert

```c
bool bloomdb_insert(BloomDB* db, const void* key, size_t len);
```

Inserts a key into the Bloom filter (simple API).

**Parameters:**
- `db`: Bloom filter
- `key`: Pointer to key data
- `len`: Length of key in bytes (must be > 0)

**Returns:** true on success, false on error

**Example:**
```c
const char* key = "example";
if (!bloomdb_insert(db, key, strlen(key))) {
    fprintf(stderr, "Insert failed\n");
}
```

### bloomdb_insert_ex

```c
BloomDBError bloomdb_insert_ex(BloomDB* db, const void* key, size_t len);
```

Inserts a key into the Bloom filter with explicit error handling.

**Parameters:**
- `db`: Bloom filter (must not be NULL)
- `key`: Pointer to key data (must not be NULL)
- `len`: Length of key in bytes (must be > 0)

**Returns:**
- `BLOOMDB_OK` on success
- `BLOOMDB_ERR_INVALID_ARGUMENT` if parameters are invalid

**Example:**
```c
BloomDBError err = bloomdb_insert_ex(db, key, strlen(key));
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Insert error: %s\n", bloomdb_strerror(err));
}
```

---

## Lookup

### bloomdb_might_contain

```c
bool bloomdb_might_contain(const BloomDB* db, const void* key, size_t len);
```

Checks if a key might be in the filter (simple API).

**Parameters:**
- `db`: Bloom filter
- `key`: Pointer to key data
- `len`: Length of key in bytes (must be > 0)

**Returns:**
- true: Key **might** be in the set (or false positive)
- false: Key is **definitely not** in the set (on success), or error occurred

**Example:**
```c
const char* key = "example";
if (bloomdb_might_contain(db, key, strlen(key))) {
    printf("Key might be present\n");
} else {
    printf("Key definitely not present\n");
}
```

### bloomdb_might_contain_ex

```c
BloomDBError bloomdb_might_contain_ex(const BloomDB* db, const void* key, size_t len, bool* out_result);
```

Checks if a key might be in the filter with explicit error handling.

**Parameters:**
- `db`: Bloom filter (must not be NULL)
- `key`: Pointer to key data (must not be NULL)
- `len`: Length of key in bytes (must be > 0)
- `out_result`: Output parameter for result (must not be NULL)

**Returns:**
- `BLOOMDB_OK` on success (`out_result` contains true/false)
- `BLOOMDB_ERR_INVALID_ARGUMENT` if parameters are invalid

**Example:**
```c
bool result;
BloomDBError err = bloomdb_might_contain_ex(db, key, strlen(key), &result);
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Lookup error: %s\n", bloomdb_strerror(err));
} else if (result) {
    printf("Key might be present\n");
} else {
    printf("Key definitely not present\n");
}
```

---

## Persistence

### bloomdb_save

```c
bool bloomdb_save(const BloomDB* db, const char* path);
```

Saves a Bloom filter to a file (simple API).

**Parameters:**
- `db`: Bloom filter to save
- `path`: File path

**Returns:** true on success, false on error

**Example:**
```c
if (!bloomdb_save(db, "filter.bloomdb")) {
    fprintf(stderr, "Save failed\n");
}
```

### bloomdb_save_ex

```c
BloomDBError bloomdb_save_ex(const BloomDB* db, const char* path);
```

Saves a Bloom filter to a file with explicit error handling.

**Parameters:**
- `db`: Bloom filter to save (must not be NULL)
- `path`: File path (must not be NULL)

**Returns:**
- `BLOOMDB_OK` on success
- `BLOOMDB_ERR_INVALID_ARGUMENT` if parameters are invalid
- `BLOOMDB_ERR_FILE_IO` if file operation fails

**Example:**
```c
BloomDBError err = bloomdb_save_ex(db, "filter.bloomdb");
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Save error: %s\n", bloomdb_strerror(err));
}
```

### bloomdb_load

```c
BloomDB* bloomdb_load(const char* path);
```

Loads a Bloom filter from a file (simple API).

**Parameters:**
- `path`: File path

**Returns:** Pointer to loaded BloomDB on success, NULL on error

**Example:**
```c
BloomDB* db = bloomdb_load("filter.bloomdb");
if (!db) {
    fprintf(stderr, "Load failed\n");
}
```

### bloomdb_load_ex

```c
BloomDBError bloomdb_load_ex(const char* path, BloomDB** out_db);
```

Loads a Bloom filter from a file with explicit error handling.

**Parameters:**
- `path`: File path (must not be NULL)
- `out_db`: Output parameter for loaded BloomDB (must not be NULL)

**Returns:**
- `BLOOMDB_OK` on success
- `BLOOMDB_ERR_INVALID_ARGUMENT` if parameters are invalid
- `BLOOMDB_ERR_FILE_IO` if file cannot be opened
- `BLOOMDB_ERR_FORMAT` if file format is invalid or corrupted
- `BLOOMDB_ERR_ALLOC` if memory allocation fails

**Example:**
```c
BloomDB* db = NULL;
BloomDBError err = bloomdb_load_ex("filter.bloomdb", &db);
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Load error: %s\n", bloomdb_strerror(err));
    return 1;
}
```

---

## Helper Functions (inline)

### C String Helpers

```c
static inline bool bloomdb_insert_cstr(BloomDB* db, const char* s);
static inline bool bloomdb_might_contain_cstr(const BloomDB* db, const char* s);
static inline BloomDBError bloomdb_insert_cstr_ex(BloomDB* db, const char* s);
static inline BloomDBError bloomdb_might_contain_cstr_ex(const BloomDB* db, const char* s, bool* out_result);
```

Convenience functions for inserting and querying C strings (uses `strlen` internally).

**Example:**
```c
bloomdb_insert_cstr(db, "hello");
if (bloomdb_might_contain_cstr(db, "hello")) {
    printf("Found!\n");
}

// With explicit error handling:
BloomDBError err = bloomdb_insert_cstr_ex(db, "world");
bool result;
err = bloomdb_might_contain_cstr_ex(db, "world", &result);
```

### uint64_t Helpers

```c
static inline bool bloomdb_insert_u64(BloomDB* db, uint64_t value);
static inline bool bloomdb_might_contain_u64(const BloomDB* db, uint64_t value);
static inline BloomDBError bloomdb_insert_u64_ex(BloomDB* db, uint64_t value);
static inline BloomDBError bloomdb_might_contain_u64_ex(const BloomDB* db, uint64_t value, bool* out_result);
```

Convenience functions for inserting and querying uint64_t values.

**Example:**
```c
uint64_t user_id = 123456789;
bloomdb_insert_u64(db, user_id);
if (bloomdb_might_contain_u64(db, user_id)) {
    printf("User ID found!\n");
}

// With explicit error handling:
BloomDBError err = bloomdb_insert_u64_ex(db, user_id);
bool result;
err = bloomdb_might_contain_u64_ex(db, user_id, &result);
```

---

## Usage Patterns

### Basic Usage

```c
// Create filter
BloomDB* db = bloomdb_create(100000, 3, 42);

// Insert keys
bloomdb_insert_cstr(db, "alice");
bloomdb_insert_cstr(db, "bob");
bloomdb_insert_u64(db, 12345);

// Query
if (bloomdb_might_contain_cstr(db, "alice")) {
    printf("alice might be present\n");
}

// Save
bloomdb_save(db, "users.bloomdb");

// Cleanup
bloomdb_free(db);
```

### Error-Aware Usage

```c
BloomDB* db = NULL;
BloomDBError err = bloomdb_create_ex(100000, 3, 42, &db);
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Create failed: %s\n", bloomdb_strerror(err));
    return 1;
}

err = bloomdb_insert_cstr_ex(db, "alice");
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Insert failed: %s\n", bloomdb_strerror(err));
    bloomdb_free(db);
    return 1;
}

bool result;
err = bloomdb_might_contain_cstr_ex(db, "alice", &result);
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Query failed: %s\n", bloomdb_strerror(err));
} else if (result) {
    printf("alice might be present\n");
}

err = bloomdb_save_ex(db, "users.bloomdb");
if (err != BLOOMDB_OK) {
    fprintf(stderr, "Save failed: %s\n", bloomdb_strerror(err));
}

bloomdb_free(db);
```

### Loading from File

```c
BloomDB* db = NULL;
BloomDBError err = bloomdb_load_ex("users.bloomdb", &db);

switch (err) {
    case BLOOMDB_OK:
        printf("Loaded successfully\n");
        break;
    case BLOOMDB_ERR_FILE_IO:
        fprintf(stderr, "File not found or cannot be opened\n");
        return 1;
    case BLOOMDB_ERR_FORMAT:
        fprintf(stderr, "Invalid or corrupted file format\n");
        return 1;
    case BLOOMDB_ERR_ALLOC:
        fprintf(stderr, "Out of memory\n");
        return 1;
    default:
        fprintf(stderr, "Unknown error: %s\n", bloomdb_strerror(err));
        return 1;
}

// Use db...
bloomdb_free(db);
```

---

## Notes

1. **False Positives:** Bloom filters can return false positives (saying a key might exist when it doesn't), but **never false negatives** (saying a key doesn't exist when it does).

2. **Parameter Sizing:** Choose `bits` and `num_hashes` based on your expected number of elements and desired false positive rate:
   - `bits = -(n * ln(p)) / (ln(2)^2)` where n = elements, p = false positive rate
   - `num_hashes = (bits / n) * ln(2)`

3. **Thread Safety:** BloomDB is **not thread-safe**. Use external synchronization if accessing from multiple threads.

4. **Binary Compatibility:** The file format uses native `size_t`, `int`, and `uint64_t` sizes. Files are **not portable** across architectures with different sizes.

5. **API Design:** Functions ending in `_ex` provide explicit error codes. Simple functions wrap `_ex` functions and return bool/NULL on error.
