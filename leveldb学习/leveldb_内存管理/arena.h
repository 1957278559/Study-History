// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_UTIL_ARENA_H_
#define STORAGE_LEVELDB_UTIL_ARENA_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {

class Arena {
public:
  Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  // Area率属于Memtable，所以当Memtable结束生命周期，析构函数会释放内存池
  ~Arena();

  // 返回指向新分配内存的指针
  char* Allocate(size_t bytes);

  // 分配偶数大小的内存，主要是skiplist节点时，目的是加速访问
  // 确保内存对齐
  char* AllocateAligned(size_t bytes);

  // Returns an estimate of the total memory usage of data allocated
  // by the arena.
  size_t MemoryUsage() const {
    return memory_usage_.load(std::memory_order_relaxed);
  }

 private:
  char* AllocateFallback(size_t bytes); // 按需分配内存，可能会有内存浪费
  char* AllocateNewBlock(size_t block_bytes);

  // 当前已使用内存的指针
  char* alloc_ptr_;

  //剩余内存字节数
  size_t alloc_bytes_remaining_;

  // 实际分配的内存池
  std::vector<char*> blocks_;

  //记录内存使用情况
  std::atomic<size_t> memory_usage_;
};

inline char* Arena::Allocate(size_t bytes) {
  // The semantics of what to return are a bit messy if we allow
  // 0-byte allocations, so we disallow them here (we don't need
  // them for our internal use).
  assert(bytes > 0);

  //优先判断当前剩余内存字节数是否足够
  //不够再通过AllocateFallback函数分配新内存
  if (bytes <= alloc_bytes_remaining_) {
    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
  }
  return AllocateFallback(bytes);
}

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_ARENA_H_
