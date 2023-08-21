#ifndef STORAGE_LEVELDB_DB_MEMTABLE_H_
#define STORAGE_LEVELDB_DB_MEMTABLE_H_

#include <string>

#include "db/dbformat.h"
#include "db/skiplist.h"
#include "leveldb/db.h"
#include "util/arena.h"

namespace leveldb {

    class InternalKeyComparator;
    class MemTableIterator;

    class MemTable {
    public:
        // MemTable 是引用计数的。初始引用计数为零，调用者必须至少调用一次 Ref()
        explicit MemTable(const InternalKeyComparator& comparator);

        MemTable(const MemTable&) = delete;
        MemTable& operator=(const MemTable&) = delete;

        // 增加引用计数
        void Ref() { ++refs_; }

        // 减少引用计数，如果没有引用存在，则删除
        void Unref() {
            --refs_;
            assert(refs_ >= 0);
            if (refs_ <= 0) {
                delete this;
            }
        }

        // 估计当前 MemTable 实例占用的内存大小
        size_t ApproximateMemoryUsage();

        // 返回一个 Iterator 指针，用于遍历 MemTable 中的键值对数据
        Iterator* NewIterator();

        // 向 MemTable 中添加一个键值对
        void Add(SequenceNumber seq, ValueType type, const Slice& key,
                const Slice& value);

        // 通过键查找对应的值 
        bool Get(const LookupKey& key, std::string* value, Status* s);

    private:
        friend class MemTableIterator;
        friend class MemTableBackwardIterator;

        struct KeyComparator {
            const InternalKeyComparator comparator;
            explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
            int operator()(const char* a, const char* b) const;
        };

        typedef SkipList<const char*, KeyComparator> Table;

        ~MemTable();  // 私有，因为只应使用 Unref() 来删除它

        KeyComparator comparator_;
        int refs_;
        Arena arena_;
        Table table_;
    };
}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_MEMTABLE_H_