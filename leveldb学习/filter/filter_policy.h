#ifndef STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_
#define STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_

#include <string>

// 外部可以为数据库配置自定义的 FilterPolicy 对象
// 该对象负责从一组键中创建一个小型过滤器
// 这些过滤器存储在 leveldb 中，并在需要从磁盘读取信息时，由 leveldb 自动进行查询
// 在许多情况下，过滤器可以将磁盘寻址的数量减少到每个 DB::Get() 调用仅需一次磁盘寻址

#include "leveldb/export.h"

namespace leveldb {

    class Slice;

    // 纯虚类，用于定义过滤策略的接口
    class LEVELDB_EXPORT FilterPolicy {
    public:
        virtual ~FilterPolicy();

        // 纯虚函数，返回过滤策略的名称
        // 如果过滤器的编码方式发生不兼容的变化，此方法返回的名称必须更改
        // 以确保旧的不兼容的过滤器不会传递给这个类型的其他方法
        virtual const char* Name() const = 0;

        // 纯虚函数，用于根据一组键创建一个过滤器
        // keys 是一个包含按用户提供的比较器排序的键的列表（可能包含重复键）
        // n 是键的数量
        // dst 是一个输出参数，用于存储新创建的过滤器
        virtual void CreateFilter(const Slice* keys, int n, std::string* dst) const = 0;

        // 纯虚函数，判定给定键是否可能存在于之前创建的过滤器中
        // key 是要检查的键，filter 是之前创建的过滤器的内容
        virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;
    };

    // 全局函数，用于创建一个使用布隆过滤器的过滤策略
    // bits_per_key 表示每个键需要的平均比特数，通常设置为 10，以获得约 1% 的错误概率
    // 返回一个指向过滤策略对象的指针
    LEVELDB_EXPORT const FilterPolicy* NewBloomFilterPolicy(int bits_per_key);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_