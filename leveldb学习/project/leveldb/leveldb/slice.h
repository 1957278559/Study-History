#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H_
#define STORAGE_LEVELDB_INCLUDE_SLICE_H_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

namespace leveldb {
	// 用于处理字符串的类
	class Slice {
	public:
		Slice() : data_(""), size_(0) {} // 创建一个空的 Slice
		Slice(const char* d, size_t n) : data_(d), size_(n) {} // 根据指定数据创建字符串切片
		Slice(const std::string& s) : data_(s.data()), size_(s.size()) {} // 使用一个 string 值来创建切片
		Slice(const char* s) : data_(s), size_(strlen(s)) {} // 使用一个 char* 值来创建切片
		Slice(const Slice&) = default;
		Slice& operator=(const Slice&) = default;

		const char* data() const { return data_; } // 返回指向数据开始的指针
		size_t size() const { return size_; } // 返回数据的长度
		bool empty() const { return size_ == 0; } // 判断数据是否为空
		char operator[](size_t n) const { assert(n < size()); return data_[n]; } // 支持下标索引
		void clear() { data_ = ""; size_ = 0; } // 清空数据
		void remove_prefix(size_t n) { assert(n <= size()); data_ += n; size_ -= n; } // 删除切片中的前 n 个字符
		std::string ToString() const { return std::string(data_, size_); } // 将切片转换为 string 类型
		bool starts_with(const Slice& x) const { return ((size_ >= x.size_) && memcmp(data_, x.data_, x.size_) == 0); } // 判断 x 切片是否是该切片的开头
		int compare(const Slice& b) const; // 比较：*this < b 返回值小于0；*this == b 返回值等于0；*this > b 返回值大于0

	private:
		const char* data_; // 数据地址
		size_t size_; // 数据长度
	};

	// 判断两个字符串切片是否相等
	inline bool operator==(const Slice& x, const Slice& y) {
		return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
	}

	// 判断两个字符串切片不相等
	inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

	// 切片比较函数
	inline int Slice::compare(const Slice& b) const {
		const size_t min_lin = (size_ < b.size_) ? size_ : b.size_;
		int r = memcmp(data_, b.data_, min_lin);
		if (r == 0) {
			if (size_ < b.size_)
				r = -1;
			else if (size_ > b.size_)
				r = +1;
		}
		return r;
	}
}

#endif // !STORAGE_LEVELDB_INCLUDE_SLICE_H_