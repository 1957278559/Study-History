#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H_
#define STORAGE_LEVELDB_INCLUDE_SLICE_H_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

#include "export.h"

namespace leveldb
{
	// Slice һ���򵥵�������Ƭ�ṹ������һ��ָ���ⲿ�洢��ָ�����Ƭ�Ĵ�С
	// ���ڱ�ʾһ�����������ݣ������ַ�������������ݣ�����ӵ�����ݵ�����Ȩ
	// �û���Ҫȷ����ʹ�� Slice ����ʱ����Ӧ���ⲿ�洢�����ڱ��ͷŻ�����ǰ��ʹ��
	// ���ڹ������ݿ��еļ�ֵ�����ݣ������ڴ���ӵ����������Ȩ�ĳ���
	// ͬʱ֧�ֿ��ٵ����ݷ��ʺͱȽϲ���
	class LEVELDB_EXPORT Slice
	{
	public:
		// ����һ���յ� Slice
		Slice() : data_(""), size_(0) {}

		// ����һ�� Slice ����ΧΪ���� d �� [0, n-1]
		Slice(const char* d, size_t n) : data_(d), size_(n) {}

		// ʹ���ַ������� Slice
		Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

		Slice(const char* s) : data_(s), size_(strlen(s)) {}

		Slice(const Slice&) = default;
		Slice& operator=(const Slice&) = default;

		// ����ָ��������ʼ��ָ��
		const char* data() const { return data_; }

		// �������ݳ���
		size_t size() const { return size_; }

		// �ж������Ƿ�Ϊ��
		bool empty() const { return size_ == 0; }

		// ���������������ݷ���
		char operator[](size_t n) const
		{
			assert(n < size());
			return data_[n];
		}

		// ��� slice ����
		void clear()
		{
			data_ = "";
			size_ = 0;
		}

		// ɾ����Ƭǰ n ������
		void remove_prefix(size_t n)
		{
			assert(n <= size());
			data_ += n;
			size_ -= n;
		}

		// ������ת��Ϊ string ������
		std::string ToString() const { return std::string(data_, size_); }

		// ������·�Ƚ�
		int compare(const Slice& b) const;

		bool starts_with(const Slice& x) const
		{
			return ((size_ >= x.size_) && (memcmp(data_, x.data_, x.size_) == 0));
		}

	private:
		const char* data_;
		size_t size_;
	};

	inline bool operator==(const Slice& x, const Slice& y)
	{
		return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
	}

	inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

	inline int Slice::compare(const Slice& b) const
	{
		const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
		int r = memcmp(data_, b.data_, min_len);
		if (r == 0)
		{
			if (size_ < b.size_)
				r = -1;
			else if (size_ > b.size_)
				r = +1;
		}
		return r;
	}

}	// namespace leveldb

#endif // !STORAGE_LEVELDB_INCLUDE_SLICE_H_