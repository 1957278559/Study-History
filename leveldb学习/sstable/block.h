#ifndef STORAGE_LEVELDB_TABLE_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_H_

#include <stddef.h>
#include <stdint.h>

#include "leveldb/iterator.h"

namespace leveldb {

    struct BlockContents;
    class Comparator;

    // 用于表示 leveldb 的数据块
    class Block {
    public:
        // 使用指定的内容初始化块。
        // BlockContents 用于存储块的实际内容和相关信息
        explicit Block(const BlockContents& contents);

        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;

        ~Block();

        // 返回块大小
        size_t size() const { return size_; }

        // 创建并返回一个新的迭代器，用于在块中遍历键值对
        // Comparator 用于比较键的顺序
        Iterator* NewIterator(const Comparator* comparator);

    private:
        class Iter;

        uint32_t NumRestarts() const;

        const char* data_;          // 指向块数据的指针
        size_t size_;               // 块数据的大小
        uint32_t restart_offset_;   // 重启数组在data_中的偏移量
        bool owned_;                // 块是否拥有数据的标志
        };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_H_