#ifndef STORAGE_LEVELDB_UTIL_HASH_H_
#define STORAGE_LEVELDB_UTIL_HASH_H_

#include <cstddef>
#include <cstdint>

namespace leveldb {
	uint32_t Hash(const char* data, size_t n, uint32_t seed);
}

#endif  // STORAGE_LEVELDB_UTIL_HASH_H_