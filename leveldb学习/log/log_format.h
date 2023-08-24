#ifndef STORAGE_LEVELDB_DB_LOG_FORMAT_H_
#define STORAGE_LEVELDB_DB_LOG_FORMAT_H_

namespace leveldb {
    namespace log {

        enum RecordType {
                // 零保留用于预分配的文件
                kZeroType = 0,

                // 完整记录类型
                kFullType = 1,

                // 用于片段的记录类型
                kFirstType = 2,
                kMiddleType = 3,
                kLastType = 4
            };
            static const int kMaxRecordType = kLastType;

            // 数据块大小
            static const int kBlockSize = 32768;

            // 头部包括校验和(4字节)、长度(2字节)、类型(1字节)
            static const int kHeaderSize = 4 + 2 + 1;

    }  // namespace log
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_LOG_FORMAT_H_