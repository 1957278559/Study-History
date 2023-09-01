#ifndef STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_
#define STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_

#include <string>

namespace leveldb {
	class Slice;

	// 用于为 leveldb 中的键(key)提供全局的比较顺序的抽象
	// 需要由派生类实现这些方法
	class Comparator {
	public:
		virtual ~Comparator();

		// 执行三路比较，比较两个 Slice 对象的内容，规则是：
		// 返回负数：如果 a 小于 b
		// 返回零：如果 a 等于 b
		// 返回正数：如果 a 大于 b
		virtual int Compare(const Slice& a, const Slice& b) const = 0;

		// 返回比较器名称的字符串
		virtual const char* Name() const = 0;

		// 用于优化内部数据结构，以减少索引块(index block)等数据结构的空间要求
		// 如果 *start < limit，函数可以将 *start 修改为一个在 [start, limit) 范围内较短的字符串
		virtual void FindShortestSeparator(std::string* start, const Slice& limit) const = 0;

		// 将 *key 修改为一个大于等于 *key 的较短字符串
		virtual void FindShortSuccessor(std::string* key) const = 0;
	};

	// 返回一个内建比较器对象的指针，按字节序进行排序
	const Comparator* BytewiseComparator();
}


#endif  // STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_