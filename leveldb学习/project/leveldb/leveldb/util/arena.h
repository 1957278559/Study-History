#ifndef STORAGE_LEVELDB_UTIL_ARENA_H_
#define STORAGE_LEVELDB_UTIL_ARENA_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {
	// 定义内存块大小
	static const int kBlockSize = 4096;

	// 用于内存分配的类
	class Arena {
	public:
		Arena() : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}
		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;
		~Arena() {
			for (size_t i = 0; i < blocks_.size(); i++)
				delete[] blocks_[i];
		}

		char* Allocate(size_t bytes); // 返回指向新分配内存的内存块的指针，他会检查是否可以从当前块中分配所需内存，如果不行，则会回退到 AllocateFallback 函数
		char* AllocateAligned(size_t bytes); // 保证使用 malloc 进行内存对齐
		size_t MemoryUsage() const { return memory_usage_.load(std::memory_order_relaxed); } // 返回分配的数据占用的总内存量的估计值

	private:
		char* alloc_ptr_; // 指向当前块中下一个可用内存位置的指针
		size_t alloc_bytes_remaining_; // 当前块中剩余字节数
		std::vector<char*> blocks_; // 分配的内存块指针的数组
		std::atomic<size_t> memory_usage_; // 原子计数器，用于跟踪总内存使用情况

		char* AllocateFallback(size_t bytes); // 如果当前块中的空间不足，此函数会分配一个新的内存块，并返回所请求字节数的指针
		char* AllocateNewBlock(size_t block_bytes); // 分配一个指定大小的新内存快，并更新分配状态
	};

	inline char* Arena::Allocate(size_t bytes) {
		// 不希望使用 0字节 的内存分配
		assert(bytes > 0);
		if (bytes <= alloc_bytes_remaining_) { // 当前块中剩余的内存足够使用
			char* result = alloc_ptr_;
			alloc_ptr_ += bytes;
			alloc_bytes_remaining_ -= bytes;
			return result;
		}
		return AllocateFallback(bytes); // 块中剩余内存不足，需要分配新内存块
	}

	char* Arena::AllocateFallback(size_t bytes) {
		// 如果需要分配的内存大于内存块的 1/4，则会创建新的内存块
		if (bytes > kBlockSize / 4) {
			char* result = AllocateNewBlock(bytes);
			return result;
		}
		// 浪费掉当前内存块剩余的空间。将 alloc_ptr_ 设置为新分配的内存块的起始地址
		// 因为当该函数被调用时，表示当前内存块剩余的内存不够使用，但需要分配的内存
		// 又小于一个内存块大小的 1/4
		alloc_ptr_ = AllocateNewBlock(kBlockSize);
		alloc_bytes_remaining_ = kBlockSize;

		char* result = alloc_ptr_;
		alloc_ptr_ += bytes;
		alloc_bytes_remaining_ -= bytes;
		return result;
	}

	char* Arena::AllocateAligned(size_t bytes) {
		// 根据操作系统和硬件的要求，计算出对齐所需要的字节数，以便分配的内存可以正确对齐
		const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
		static_assert((align & (align - 1)) == 0, "Pointer size should be a power of 2");
		size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
		size_t slop = (current_mod == 0 ? 0 : align - current_mod);
		size_t needed = bytes + slop;
		char* result;
		if (needed <= alloc_bytes_remaining_) { // 所需要的字节数小于当前剩余的字节数
			result = alloc_ptr_ + slop;
			alloc_ptr_ += needed;
			alloc_bytes_remaining_ -= needed;
		}
		else 
			result = AllocateFallback(bytes);
		assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
		return result;
	}

	char* Arena::AllocateNewBlock(size_t block_bytes) {
		char* result = new char[block_bytes];
		blocks_.push_back(result); // 添加到数组中
		memory_usage_.fetch_add(block_bytes + sizeof(char*), std::memory_order_relaxed); // 通过原子操作将分配的内存块大小添加到计数器中，以便跟踪总内存的使用情况
		return result;
	}
}

#endif  // STORAGE_LEVELDB_UTIL_ARENA_H_