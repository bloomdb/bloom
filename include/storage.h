#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>

#include "bloomdb.h"

bool     bloomdb_save(const BloomDB* db, const char* path);
BloomDB* bloomdb_load(const char* path);

#endif
