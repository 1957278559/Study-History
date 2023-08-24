#ifndef STORAGE_LEVELDB_DB_LOG_WRITER_H_
#define STORAGE_LEVELDB_DB_LOG_WRITER_H_

#include <stdint.h>

#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

    class WritableFile;

    // 日志写入器类
    namespace log {

        class Writer {
        public:
             // 创建一个写入器，将数据追加到 "*dest"。
            // "*dest"必须最初为空。
            // 在使用此写入器时，"*dest"必须保持活动。
            explicit Writer(WritableFile* dest);

            // 创建一个写入器，将数据追加到 "*dest"。
            // "*dest"必须具有初始长度 "dest_length"。
            // 在使用此写入器时，"*dest"必须保持活动。
            Writer(WritableFile* dest, uint64_t dest_length);

            Writer(const Writer&) = delete;
            Writer& operator=(const Writer&) = delete;

            ~Writer();  

            // 讲述添加为日志记录。传入 Slice 表示带写入的数据，函数返回 Status 表示写入操作的结果 
            Status AddRecord(const Slice& slice);

        private:
            // 用于将物理记录（带有头部和数据）写入日志文件，接受记录类型、指向数据的指针和数据的长度
            Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

            WritableFile* dest_;    // 表示写入的目标文件
            int block_offset_;  // 当前块中的偏移量，用于控制数据写入的位置

            // 用于预先计算不同类型的激流的 CRC32 校验和，以减少在实际写入记录时的开销
            uint32_t type_crc_[kMaxRecordType + 1];
        };
    }  // namespace log
}  // namespace leveldb
#endif  // STORAGE_LEVELDB_DB_LOG_WRITER_H_