#ifndef STORAGE_LEVELDB_UTIL_CODING_H_
#define STORAGE_LEVELDB_UTIL_CODING_H_

#include <cstdint>
#include <cstring>
#include <string>
#include "../include/slice.h"

namespace leveldb {
	// ���� ���� �� ���� ��ʵ�ú����Ķ���
	// ��Щ�����������ڲ����ݽṹ��ִ�й̶��������֡��䳤�����Լ��ַ����ı���ͽ������
	
	// ���º������ڽ���ͬ���͵����ݱ��벢���ӵ�һ���ַ�����
	void PutFixed32(std::string* dst, uint32_t value);
	void PutFixed64(std::string* dst, uint64_t value);
	void PutVarint32(std::string* dst, uint32_t value);
	void PutVarint64(std::string* dst, uint64_t value);
	void PutLengthPrefixedSlice(std::string* dst, const Slice& value);

	// ���º������ڴ���Ƭ�н������ͬ���͵�����
	bool GetVarint32(Slice* input, uint32_t* value);
	bool GetVarint64(Slice* input, uint64_t* value);
	bool GetLengthPrefixedSlice(Slice* input, Slice* result);

	// ָ��汾�Ľ��뺯��������ֱ�����ַ���������ִ�н������
	const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v);
	const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* v);

	// ����һ���޷��� 64 λ������������ varint ������ռ�õ��ֽ���
	int VarintLength(uint64_t v);

	// �ͼ���汾�ı��뺯����ֱ�ӽ����ݱ��뵽�ַ���������������ָ����������֮��λ�õ�ָ��
	char* EncodeVarint32(char* dst, uint32_t value);
	char* EncodeVarint64(char* dst, uint64_t value);

	// �� 32 λ�� 64 λ�̶���������ֱ�ӱ��뵽�ַ�������
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

	// ���ַ�����������ֱ�ӽ��� 32 λ�� 64 λ�̶���������
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

	// ָ�� GetVarint32Ptr �Ļ��˺��������ڴ����ض�����µ� varint ����
	const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value);

	// �������������ڽ��� varint32 ���ݣ������ض����ʹ�û��˺�����
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