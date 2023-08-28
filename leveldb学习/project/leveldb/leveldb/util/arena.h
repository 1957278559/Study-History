#ifndef STORAGE_LEVELDB_UTIL_ARENA_H_
#define STORAGE_LEVELDB_UTIL_ARENA_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {
	// �����ڴ���С
	static const int kBlockSize = 4096;

	// �����ڴ�������
	class Arena {
	public:
		Arena() : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}
		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;
		~Arena() {
			for (size_t i = 0; i < blocks_.size(); i++)
				delete[] blocks_[i];
		}

		char* Allocate(size_t bytes); // ����ָ���·����ڴ���ڴ���ָ�룬�������Ƿ���Դӵ�ǰ���з��������ڴ棬������У������˵� AllocateFallback ����
		char* AllocateAligned(size_t bytes); // ��֤ʹ�� malloc �����ڴ����
		size_t MemoryUsage() const { return memory_usage_.load(std::memory_order_relaxed); } // ���ط��������ռ�õ����ڴ����Ĺ���ֵ

	private:
		char* alloc_ptr_; // ָ��ǰ������һ�������ڴ�λ�õ�ָ��
		size_t alloc_bytes_remaining_; // ��ǰ����ʣ���ֽ���
		std::vector<char*> blocks_; // ������ڴ��ָ�������
		std::atomic<size_t> memory_usage_; // ԭ�Ӽ����������ڸ������ڴ�ʹ�����

		char* AllocateFallback(size_t bytes); // �����ǰ���еĿռ䲻�㣬�˺��������һ���µ��ڴ�飬�������������ֽ�����ָ��
		char* AllocateNewBlock(size_t block_bytes); // ����һ��ָ����С�����ڴ�죬�����·���״̬
	};

	inline char* Arena::Allocate(size_t bytes) {
		// ��ϣ��ʹ�� 0�ֽ� ���ڴ����
		assert(bytes > 0);
		if (bytes <= alloc_bytes_remaining_) { // ��ǰ����ʣ����ڴ��㹻ʹ��
			char* result = alloc_ptr_;
			alloc_ptr_ += bytes;
			alloc_bytes_remaining_ -= bytes;
			return result;
		}
		return AllocateFallback(bytes); // ����ʣ���ڴ治�㣬��Ҫ�������ڴ��
	}

	char* Arena::AllocateFallback(size_t bytes) {
		// �����Ҫ������ڴ�����ڴ��� 1/4����ᴴ���µ��ڴ��
		if (bytes > kBlockSize / 4) {
			char* result = AllocateNewBlock(bytes);
			return result;
		}
		// �˷ѵ���ǰ�ڴ��ʣ��Ŀռ䡣�� alloc_ptr_ ����Ϊ�·�����ڴ�����ʼ��ַ
		// ��Ϊ���ú���������ʱ����ʾ��ǰ�ڴ��ʣ����ڴ治��ʹ�ã�����Ҫ������ڴ�
		// ��С��һ���ڴ���С�� 1/4
		alloc_ptr_ = AllocateNewBlock(kBlockSize);
		alloc_bytes_remaining_ = kBlockSize;

		char* result = alloc_ptr_;
		alloc_ptr_ += bytes;
		alloc_bytes_remaining_ -= bytes;
		return result;
	}

	char* Arena::AllocateAligned(size_t bytes) {
		// ���ݲ���ϵͳ��Ӳ����Ҫ�󣬼������������Ҫ���ֽ������Ա������ڴ������ȷ����
		const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
		static_assert((align & (align - 1)) == 0, "Pointer size should be a power of 2");
		size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
		size_t slop = (current_mod == 0 ? 0 : align - current_mod);
		size_t needed = bytes + slop;
		char* result;
		if (needed <= alloc_bytes_remaining_) { // ����Ҫ���ֽ���С�ڵ�ǰʣ����ֽ���
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
		blocks_.push_back(result); // ��ӵ�������
		memory_usage_.fetch_add(block_bytes + sizeof(char*), std::memory_order_relaxed); // ͨ��ԭ�Ӳ�����������ڴ���С��ӵ��������У��Ա�������ڴ��ʹ�����
		return result;
	}
}

#endif  // STORAGE_LEVELDB_UTIL_ARENA_H_