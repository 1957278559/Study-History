#ifndef STORAGE_LEVELDB_DB_LOG_READER_H_
#define STORAGE_LEVELDB_DB_LOG_READER_H_

#include <stdint.h>

#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

    class SequentialFile;

    namespace log {

        // 日志读取类
        class Reader {
        public:
            // 用于报告错误的接口
            class Reporter {
            public:
                virtual ~Reporter();

                // 用于报告检测到的损坏情况
                virtual void Corruption(size_t bytes, const Status& status) = 0;
            };

            // 构造函数接受一个指向顺序文件(SequentialFile)的指针，一个用于接收报告错误的指针
            // 一个布尔值表示是否校验日志记录的校验和，以及初始读取位置的偏移量
            Reader(SequentialFile* file, Reporter* reporter, bool checksum, uint64_t initial_offset);

            Reader(const Reader&) = delete;
            Reader& operator=(const Reader&) = delete;

            ~Reader();

            // 用于读取下一个日志记录
            // 参数是一个用于存储读取的记录内容的 Slice 对象，以及一个用于临时存储的 string 对象
            // 函数返回布尔值，表示是否读取成功
            bool ReadRecord(Slice* record, std::string* scratch);

            // 返回上一个读取日志记录的偏移量
            uint64_t LastRecordOffset();

        private:
            // 使用以下特殊值扩展记录类型
            enum {
                kEof = kMaxRecordType + 1,
                // 当我们发现无效的物理记录时返回。目前有三种情况会发生这种情况：
                // * 记录具有无效的CRC（ReadPhysicalRecord 报告丢弃）
                // * 记录是0长度的记录（不报告丢弃）
                // * 记录在构造函数的初始偏移下方（不报告丢弃）
                kBadRecord = kMaxRecordType + 2
            };

            // 跳过所有在 "initial_offset_" 之前完全的块。
            bool SkipToInitialBlock();

            // 读取物理记录的类型，或前面的特殊值之一
            unsigned int ReadPhysicalRecord(Slice* result);

            // 报告损坏的字节给报告器。
            // 在调用之前，必须更新 buffer_ 以删除丢弃的字节。
            void ReportCorruption(uint64_t bytes, const char* reason);
            void ReportDrop(uint64_t bytes, const Status& reason);

            SequentialFile* const file_;    // 文件指针
            Reporter* const reporter_;  // 报告期指针
            bool const checksum_;   // 是否校验和
            char* const backing_store_;     // 存储底层数据的缓冲区
            Slice buffer_;  // 读取的缓冲区
            bool eof_;  // 上一个 Read() 返回 < kBlockSize 表示已到达文件末尾

            // 上一个由 ReadRecord 返回的记录的物理偏移。
            uint64_t last_record_offset_;

            // 缓冲区末尾之后的第一个位置的偏移量。
            uint64_t end_of_buffer_offset_;

            // Offset at which to start looking for the first record to return
            uint64_t const initial_offset_;

            // 如果我们在搜索之后重新同步（initial_offset_ > 0），则为 true。
            // 特别地，在此模式下可以静默跳过 kMiddleType 和 kLastType 记录的运行。
            bool resyncing_;
        };
    }  // namespace log
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_LOG_READER_H_