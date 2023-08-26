#ifndef STORAGE_LEVELDB_DB_VERSION_EDIT_H_
#define STORAGE_LEVELDB_DB_VERSION_EDIT_H_

#include <set>
#include <utility>
#include <vector>

#include "db/dbformat.h"

namespace leveldb {

    class VersionSet;

    // FileMetaData用于存储有关文件的元数据信息
    struct FileMetaData {
        FileMetaData() : refs(0), allowed_seeks(1 << 30), file_size(0) {}

        int refs;               // 引用计数
        int allowed_seeks;      // 允许的查找次数（直到压缩）
        uint64_t number;        // 文件编号
        uint64_t file_size;     // 文件大小（字节数）
        InternalKey smallest;   // 表中最小的内部键
        InternalKey largest;    // 表中最大的内部键
    };

    // VersionEdit用于表示版本的编辑操作，用于描述如何从一个版本转换到另一个版本
    class VersionEdit {
    public:
        VersionEdit() { Clear(); }
        ~VersionEdit() {}

        void Clear();

        // 设置比较器的名称
        void SetComparatorName(const Slice& name) {
            has_comparator_ = true;
            comparator_ = name.ToString();
        }

        // 设置日志文件编号
        void SetLogNumber(uint64_t num) {
            has_log_number_ = true;
            log_number_ = num;
        }

        // 设置上一个日志文件编号
        void SetPrevLogNumber(uint64_t num) {
            has_prev_log_number_ = true;
            prev_log_number_ = num;
        }

        // 设置下一个文件编号
        void SetNextFile(uint64_t num) {
            has_next_file_number_ = true;
            next_file_number_ = num;
        }

        // 设置最后的序列号
        void SetLastSequence(SequenceNumber seq) {
            has_last_sequence_ = true;
            last_sequence_ = seq;
        }

        // 设置压缩指针
        void SetCompactPointer(int level, const InternalKey& key) {
            compact_pointers_.push_back(std::make_pair(level, key));
        }

        // 添加指定编号的文件到指定级别
        // 要求：该版本未保存（参见VersionSet::SaveTo）
        // 要求："smallest"和"largest"是文件中最小和最大的键
        void AddFile(int level, uint64_t file, uint64_t file_size,
                    const InternalKey& smallest, const InternalKey& largest) {
            FileMetaData f;
            f.number = file;
            f.file_size = file_size;
            f.smallest = smallest;
            f.largest = largest;
            new_files_.push_back(std::make_pair(level, f));
        }

        // 从指定级别删除指定编号的文件
        void DeleteFile(int level, uint64_t file) {
            deleted_files_.insert(std::make_pair(level, file));
        }

        // 将编辑操作编码为字符串
        void EncodeTo(std::string* dst) const;

        // 从字符串解码编辑操作
        Status DecodeFrom(const Slice& src);

        // 返回调试信息的字符串表示
        std::string DebugString() const;

    private:
        friend class VersionSet;

        typedef std::set<std::pair<int, uint64_t> > DeletedFileSet;

        std::string comparator_;
        uint64_t log_number_;
        uint64_t prev_log_number_;
        uint64_t next_file_number_;
        SequenceNumber last_sequence_;
        bool has_comparator_;
        bool has_log_number_;
        bool has_prev_log_number_;
        bool has_next_file_number_;
        bool has_last_sequence_;

        std::vector<std::pair<int, InternalKey> > compact_pointers_;
        DeletedFileSet deleted_files_;
        std::vector<std::pair<int, FileMetaData> > new_files_;
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_VERSION_EDIT