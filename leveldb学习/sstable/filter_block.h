#ifndef STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "leveldb/slice.h"
#include "util/hash.h"

namespace leveldb {

    /* 过滤器块的作用是为了在进行数据检索时可以快速确定某个键是否可能存在于特定的数据块中，
    *  从而提高查询效率。这在大型数据集中的数据查找操作中非常有用
    */

    // 一个抽象类，用于指定过略策略，例如布隆过滤器
    class FilterPolicy;

    // 用于构建过滤器块。在 leveldb 表格中
    // 过滤器块位于文件末尾，它包含了所有数据块的过滤器的组合
    class FilterBlockBuilder {
    public:
        // 接受一个 FilterPolicy 的指针，指定过滤策略
        explicit FilterBlockBuilder(const FilterPolicy*);

        FilterBlockBuilder(const FilterBlockBuilder&) = delete;
        FilterBlockBuilder& operator=(const FilterBlockBuilder&) = delete;

        // 开始构建一个数据块的过滤器，block_offset 表示数据块的偏移量
        void StartBlock(uint64_t block_offset);

        // 将键添加到当前数据的过滤器中
        void AddKey(const Slice& key);

        // 完成构建过滤器，返回过滤器块的内容
        Slice Finish();

    private:
        void GenerateFilter();

        const FilterPolicy* policy_;
        std::string keys_;             // 扁平化的键内容
        std::vector<size_t> start_;    // 每个键在keys_中的起始索引
        std::string result_;           // 到目前为止计算的过滤器数据
        std::vector<Slice> tmp_keys_;  // policy_->CreateFilter()的参数
        std::vector<uint32_t> filter_offsets_;
    };

    // 用于读取过滤器块，以检查特定的键是否可能存在于特定
    class FilterBlockReader {
    public:
        // 接受一个 FilterPolicy 的指针和过滤器块内容的 Slice。
        FilterBlockReader(const FilterPolicy* policy, const Slice& contents);

        // 检查给定的键是否可能存在于位于指定偏移量的数据块中
        bool KeyMayMatch(uint64_t block_offset, const Slice& key);

    private:
        const FilterPolicy* policy_;
        const char* data_;    // 指向过滤器数据的指针（在块的起始处）
        const char* offset_;  // 指向偏移数组的开头（在块的末尾）
        size_t num_;          // 偏移数组中的条目数
        size_t base_lg_;      // 编码参数（参见.cc文件中的kFilterBaseLg）
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_