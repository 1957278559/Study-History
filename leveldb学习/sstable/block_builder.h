#ifndef STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_

#include <stdint.h>

#include <vector>

#include "leveldb/slice.h"

namespace leveldb {

    struct Options;

    // 用于构建 leveldb 数据块。这些数据块用于将键值对存储在 leveldb 表格中
    class BlockBuilder {
    public:
        explicit BlockBuilder(const Options* options);

        BlockBuilder(const BlockBuilder&) = delete;
        BlockBuilder& operator=(const BlockBuilder&) = delete;

        // 将内容重置，就像BlockBuilder刚刚构造一样。
        void Reset();

        // 添加一个键值对，要求添加的键大于先前添加的任何键
        void Add(const Slice& key, const Slice& value);

        // 完成块的构建并返回一个指向块内容的切片
        Slice Finish();

        // 返回当前正在构建的块的（未压缩的）当前大小的估计
        size_t CurrentSizeEstimate() const;

        // 当前是否没有自上次Reset()以来添加任何条目
        bool empty() const { return buffer_.empty(); }

    private:
        const Options* options_;
        std::string buffer_;                // 目标缓冲区
        std::vector<uint32_t> restarts_;    // 重启点
        int counter_;                       // 自重启以来发出的条目数
        bool finished_;                     //  是否已调用Finish()？
        std::string last_key_;
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_