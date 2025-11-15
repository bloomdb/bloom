#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>

#include "bloomdb.h"

// ============================================================================
// Simple API (returns false/NULL on error)
// ============================================================================

bool     bloomdb_save(const BloomDB* db, const char* path);
BloomDB* bloomdb_load(const char* path);

// ============================================================================
// Extended API (explicit error handling)
// ============================================================================

BloomDBError bloomdb_save_ex(const BloomDB* db, const char* path);
BloomDBError bloomdb_load_ex(const char* path, BloomDB** out_db);

#endif
