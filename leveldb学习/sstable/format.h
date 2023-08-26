#ifndef STORAGE_LEVELDB_TABLE_FORMAT_H_
#define STORAGE_LEVELDB_TABLE_FORMAT_H_

#include <stdint.h>

#include <string>

#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "leveldb/table_builder.h"

namespace leveldb {

    class Block;
    class RandomAccessFile;
    struct ReadOptions;

    // BlockHandle是一个用于指向数据块或袁术块在文件中存储范围的类
    class BlockHandle {
    public:
        // BlockHandle的最大编码长度
        enum { kMaxEncodedLength = 10 + 10 };

        BlockHandle();

        // 块在文件中的偏移量。
        uint64_t offset() const { return offset_; }
        void set_offset(uint64_t offset) { offset_ = offset; }

        // 存储块的大小
        uint64_t size() const { return size_; }
        void set_size(uint64_t size) { size_ = size; }

        void EncodeTo(std::string* dst) const;
        Status DecodeFrom(Slice* input);

    private:
        uint64_t offset_;   // 偏移量表示在文件中的位置
        uint64_t size_;     // 大小表示块的字节大小
    };

    // Footer封装了每个表文件尾部存储的固定信息。
    class Footer {
    public:
        // Footer的编码长度。注意，Footer的序列化始终将恰好占用这么多字节。
        // 它由两个块句柄和一个魔术数组成。
        enum { kEncodedLength = 2 * BlockHandle::kMaxEncodedLength + 8 };

        Footer() {}

        // 表的元索引块的块句柄
        const BlockHandle& metaindex_handle() const { return metaindex_handle_; }
        void set_metaindex_handle(const BlockHandle& h) { metaindex_handle_ = h; }

        // 表的索引块的块句柄
        const BlockHandle& index_handle() const { return index_handle_; }
        void set_index_handle(const BlockHandle& h) { index_handle_ = h; }

        void EncodeTo(std::string* dst) const; // 用于将 BlockHandle 和 Footer 编码成字节流
        Status DecodeFrom(Slice* input);    // 用于将 BlockHandle 和 Footer 从字节流解码回来

    private:
        BlockHandle metaindex_handle_;  // 指向元索引块
        BlockHandle index_handle_;  // 指向索引块
    };

    // kTableMagicNumber was picked by running
    //    echo http://code.google.com/p/leveldb/ | sha1sum
    // and taking the leading 64 bits.
    static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;    // 用于标识 leveldb 表格的魔术数字

    // 1字节类型 + 32位crc
    static const size_t kBlockTrailerSize = 5;  // 表示块尾部的大小

    struct BlockContents {  // 用于存储块的内容，是否可缓存以及是否需要在堆上分配内存
        Slice data;           // 数据的实际内容
        bool cachable;        // 数据是否可以被缓存
        bool heap_allocated;  // 调用方是否应该删除[] data.data()
        };

    // 从“file”中读取由“handle”标识的块。如果失败，则返回非OK
    // 如果成功，则填充*result并返回OK。
    // 用于从文件中读取一个块
    Status ReadBlock(RandomAccessFile* file, const ReadOptions& options, const BlockHandle& handle, BlockContents* result);

    // 后面是实现细节。客户端应忽略，

    inline BlockHandle::BlockHandle()
        : offset_(~static_cast<uint64_t>(0)), size_(~static_cast<uint64_t>(0)) {}

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_FORMAT_H_