#ifndef STORAGE_LEVELDB_UTIL_CODING_H_
#define STORAGE_LEVELDB_UTIL_CODING_H_

#include <cstdint>
#include <cstring>
#include <string>
#include "../include/slice.h"

namespace leveldb {
	// 关于 编码 和 解码 的实用函数的定义
	// 这些函数用于在内部数据结构中执行固定长度数字、变长整数以及字符串的编码和解码操作
	
	// 以下函数用于将不同类型的数据编码并附加到一个字符串中
	void PutFixed32(std::string* dst, uint32_t value);
	void PutFixed64(std::string* dst, uint64_t value);
	void PutVarint32(std::string* dst, uint32_t value);
	void PutVarint64(std::string* dst, uint64_t value);
	void PutLengthPrefixedSlice(std::string* dst, const Slice& value);

	// 以下函数用于从切片中解码出不同类型的数据
	bool GetVarint32(Slice* input, uint32_t* value);
	bool GetVarint64(Slice* input, uint64_t* value);
	bool GetLengthPrefixedSlice(Slice* input, Slice* result);

	// 指针版本的解码函数，可以直接在字符缓冲区上执行解码操作
	const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v);
	const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* v);

	// 给定一个无符号 64 位整数，返回其 varint 编码所占用的字节数
	int VarintLength(uint64_t v);

	// 低级别版本的编码函数，直接将数据编码到字符缓冲区，并返回指向编码后数据之后位置的指针
	char* EncodeVarint32(char* dst, uint32_t value);
	char* EncodeVarint64(char* dst, uint64_t value);

	// 将 32 位和 64 位固定长度整数直接编码到字符缓冲区
	inline void EncodeFixed32(char* dst, uint32_t value) 
	{
		uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);

		// Recent clang and gcc optimize this to a single mov / str instruction.
		buffer[0] = static_cast<uint8_t>(value);
		buffer[1] = static_cast<uint8_t>(value >> 8);
		buffer[2] = static_cast<uint8_t>(value >> 16);
		buffer[3] = static_cast<uint8_t>(value >> 24);
	}
	inline void EncodeFixed64(char* dst, uint64_t value)
	{
		uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);

		// Recent clang and gcc optimize this to a single mov / str instruction.
		buffer[0] = static_cast<uint8_t>(value);
		buffer[1] = static_cast<uint8_t>(value >> 8);
		buffer[2] = static_cast<uint8_t>(value >> 16);
		buffer[3] = static_cast<uint8_t>(value >> 24);
		buffer[4] = static_cast<uint8_t>(value >> 32);
		buffer[5] = static_cast<uint8_t>(value >> 40);
		buffer[6] = static_cast<uint8_t>(value >> 48);
		buffer[7] = static_cast<uint8_t>(value >> 56);
	}

	// 从字符串缓冲区中直接解码 32 位和 64 位固定长度整数
	inline uint32_t DecodeFixed32(const char* ptr)
	{
		const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);

		// Recent clang and gcc optimize this to a single mov / ldr instruction.
		return (static_cast<uint32_t>(buffer[0])) |
			(static_cast<uint32_t>(buffer[1]) << 8) |
			(static_cast<uint32_t>(buffer[2]) << 16) |
			(static_cast<uint32_t>(buffer[3]) << 24);
	}
	inline uint64_t DecodeFixed64(const char* ptr)
	{
		const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);

		// Recent clang and gcc optimize this to a single mov / ldr instruction.
		return (static_cast<uint64_t>(buffer[0])) |
			(static_cast<uint64_t>(buffer[1]) << 8) |
			(static_cast<uint64_t>(buffer[2]) << 16) |
			(static_cast<uint64_t>(buffer[3]) << 24) |
			(static_cast<uint64_t>(buffer[4]) << 32) |
			(static_cast<uint64_t>(buffer[5]) << 40) |
			(static_cast<uint64_t>(buffer[6]) << 48) |
			(static_cast<uint64_t>(buffer[7]) << 56);
	}

	// 指向 GetVarint32Ptr 的回退函数，用于处理特定情况下的 varint 解码
	const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value);

	// 内联函数，用于解码 varint32 数据，根据特定情况使用回退函数。
	inline const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* value) 
	{
		if (p < limit) {
			uint32_t result = *(reinterpret_cast<const uint8_t*>(p));
			if ((result & 128) == 0) {
				*value = result;
				return p + 1;
			}
		}
		return GetVarint32PtrFallback(p, limit, value);
	}
}
#endif  // STORAGE_LEVELDB_UTIL_CODING_H_