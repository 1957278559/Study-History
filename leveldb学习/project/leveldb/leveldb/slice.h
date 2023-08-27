#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H_
#define STORAGE_LEVELDB_INCLUDE_SLICE_H_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

namespace leveldb {
	// ���ڴ����ַ�������
	class Slice {
	public:
		Slice() : data_(""), size_(0) {} // ����һ���յ� Slice
		Slice(const char* d, size_t n) : data_(d), size_(n) {} // ����ָ�����ݴ����ַ�����Ƭ
		Slice(const std::string& s) : data_(s.data()), size_(s.size()) {} // ʹ��һ�� string ֵ��������Ƭ
		Slice(const char* s) : data_(s), size_(strlen(s)) {} // ʹ��һ�� char* ֵ��������Ƭ
		Slice(const Slice&) = default;
		Slice& operator=(const Slice&) = default;

		const char* data() const { return data_; } // ����ָ�����ݿ�ʼ��ָ��
		size_t size() const { return size_; } // �������ݵĳ���
		bool empty() const { return size_ == 0; } // �ж������Ƿ�Ϊ��
		char operator[](size_t n) const { assert(n < size()); return data_[n]; } // ֧���±�����
		void clear() { data_ = ""; size_ = 0; } // �������
		void remove_prefix(size_t n) { assert(n <= size()); data_ += n; size_ -= n; } // ɾ����Ƭ�е�ǰ n ���ַ�
		std::string ToString() const { return std::string(data_, size_); } // ����Ƭת��Ϊ string ����
		bool starts_with(const Slice& x) const { return ((size_ >= x.size_) && memcmp(data_, x.data_, x.size_) == 0); } // �ж� x ��Ƭ�Ƿ��Ǹ���Ƭ�Ŀ�ͷ
		int compare(const Slice& b) const; // �Ƚϣ�*this < b ����ֵС��0��*this == b ����ֵ����0��*this > b ����ֵ����0

	private:
		const char* data_; // ���ݵ�ַ
		size_t size_; // ���ݳ���
	};

	// �ж������ַ�����Ƭ�Ƿ����
	inline bool operator==(const Slice& x, const Slice& y) {
		return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
	}

	// �ж������ַ�����Ƭ�����
	inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

	// ��Ƭ�ȽϺ���
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