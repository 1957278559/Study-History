#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H_
#define STORAGE_LEVELDB_INCLUDE_SLICE_H_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

#include "export.h"

namespace leveldb
{
	// Slice 一个简单的数据切片结构，包含一个指向外部存储的指针和切片的大小
	// 用于表示一段连续的数据，例如字符串或二进制数据，但不拥有数据的所有权
	// 用户需要确保在使用 Slice 对象时，对应的外部存储不会在被释放或销毁前被使用
	// 用于管理数据库中的键值对数据，适用于处理不拥有数据所有权的场景
	// 同时支持快速的数据访问和比较操作
	class LEVELDB_EXPORT Slice
	{
	public:
		// 创建一个空的 Slice
		Slice() : data_(""), size_(0) {}

		// 创建一个 Slice ，范围为变量 d 的 [0, n-1]
		Slice(const char* d, size_t n) : data_(d), size_(n) {}

		// 使用字符串创建 Slice
		Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

		Slice(const char* s) : data_(s), size_(strlen(s)) {}

		Slice(const Slice&) = default;
		Slice& operator=(const Slice&) = default;

		// 返回指向数据起始的指针
		const char* data() const { return data_; }

		// 返回数据长度
		size_t size() const { return size_; }

		// 判断数据是否为空
		bool empty() const { return size_ == 0; }

		// 根据索引进行数据访问
		char operator[](size_t n) const
		{
			assert(n < size());
			return data_[n];
		}

		// 清空 slice 数据
		void clear()
		{
			data_ = "";
			size_ = 0;
		}

		// 删除切片前 n 个数据
		void remove_prefix(size_t n)
		{
			assert(n <= size());
			data_ += n;
			size_ -= n;
		}

		// 将数据转换为 string 并返回
		std::string ToString() const { return std::string(data_, size_); }

		// 数据三路比较
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