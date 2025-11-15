# Architecture

## System Overview

BloomDB Phase 1 implements a modular Bloom filter system with clear separation of concerns across four primary modules and supporting infrastructure.

## Module Hierarchy

```
BloomDB Core
├── bitarray      (Low-level bit manipulation)
├── hash64        (Hash function implementation)
├── bloomdb       (Main Bloom filter logic)
└── storage       (Persistence layer)
```

## Design Principles

### Modularity
Each component is isolated in its own source file with a corresponding header. This enables:
- Independent testing of each module
- Easy replacement of implementations (e.g., swapping hash functions)
- Clear dependency graph
- Simplified debugging

### Single Responsibility
- `bitarray`: Only handles bit-level operations
- `hash64`: Only computes hash values
- `bloomdb`: Orchestrates the filter logic
- `storage`: Only manages file I/O

### Data Flow

```
Insert Operation:
Key → hash64() → double_hashing → bitarray_set() → Memory

Query Operation:
Key → hash64() → double_hashing → bitarray_get() → Result

Persistence:
BloomDB → storage → Binary File
Binary File → storage → BloomDB
```

## Memory Layout

### BloomDB Structure
```c
typedef struct {
    uint8_t* bitarray;   // Compressed bit array
    size_t bit_count;    // Total number of bits
    size_t byte_count;   // Total bytes used
    int num_hashes;      // Number of hash functions (k)
    uint64_t seed;       // Hash seed for reproducibility
} BloomDB;
```

### Bit Array Organization
Bits are stored in a contiguous byte array where:
- Bit index `i` is located in byte `i / 8`
- Bit position within byte is `i % 8`
- Layout is little-endian within each byte

## File Format

Binary persistence format (`.bloomdb`):
```
[bit_count: 8 bytes]
[byte_count: 8 bytes]
[num_hashes: 4 bytes]
[seed: 8 bytes]
[bitarray: byte_count bytes]
```

This format ensures:
- Fast loading without parsing
- Portable across platforms with matching endianness
- Complete state preservation

## Dependency Graph

```
main.c
  └── bloomdb.h
       ├── bitarray.h
       ├── hash64.h
       └── storage.h
            └── bloomdb.h (for BloomDB type)

tests/*
  └── (same dependencies as main)

benchmark_pro.c
  └── (all module headers)
```

## Error Handling

Current approach:
- NULL returns for allocation failures
- Boolean returns for operation success/failure
- Silent failure on invalid input (defensive checks)

Future phases will add explicit error codes and error messages.

## Thread Safety

Phase 1 implementation is single-threaded. No synchronization primitives are used. Concurrent access requires external locking.

## Extensibility Points

The architecture allows for easy extension:
- Hash function: Replace `hash64.c` implementation
- Storage backend: Modify `storage.c` for different formats
- Bit array: Optimize `bitarray.c` without affecting other modules
- Filter variants: Extend `bloomdb.c` for counting, scalable, etc.
