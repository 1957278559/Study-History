#ifndef STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_
#define STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_

#include <string>

namespace leveldb {
	class Slice;

	// 过略策略的定义和实现
	// 过滤策略用于创建一个小型过滤器，以从一组键中生成过滤器
	// 这些过滤器存储在 leveldb 中，并且在 leveldb 中自动根据过滤器来决定是否从磁盘读取某些信息
	// 通常情况下，过滤器可以将磁盘查找的数量从多次减少到每次 DB::Get() 调用只有一次磁盘查找
	class FilterPolicy {
	public:
		virtual ~FilterPolicy() {}
		virtual const char* Name() const = 0; // 纯虚函数，用于获取过略策略的名称
		virtual void CreateFilter(const Slice* keys, int n, std::string* dst) const = 0; // 根据一组键创建一个过滤器
		virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0; // 判断某个键是否可能匹配过滤器
	};

	//const FilterPolicy* NewBloomFilterPolicy(int bits_per_key); // 用于创建一个基于 Bloom 过滤器的新过滤策略，函数返回一个指向 FilterPolicy 的指针
}
#endif  // STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_