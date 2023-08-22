#include "db/memtable.h"
#include "db/dbformat.h"
#include "leveldb/comparator.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"
#include "util/coding.h"

namespace leveldb {

    // 从给定的字符数据中获取一个长度前缀的 Slice（切片）。
    // 首先从数据中解析出一个 32 位无符号整数作为长度，然后使用这个长度来构造切片
    static Slice GetLengthPrefixedSlice(const char* data) {
        uint32_t len;
        const char* p = data;
        p = GetVarint32Ptr(p, p + 5, &len);  // +5: we assume "p" is not corrupted
        return Slice(p, len);
    }

    // 构造函数接受一个 InternalKeyComparator 类型的参数，用于比较内部键
    // 内存表中的数据存储在 table_ 的数据结构中，使用 InternalKeyComparator 进行排序
    // refs_ 被用于跟踪内存表的引用计数，arena_ 用于分配内存
    MemTable::MemTable(const InternalKeyComparator& comparator) : comparator_(comparator), refs_(0), table_(comparator_, &arena_) {}

    MemTable::~MemTable() { assert(refs_ == 0); }

    // 用于估计内存表的大致内存使用量，返回 arena_ 的内存使用量
    size_t MemTable::ApproximateMemoryUsage() { return arena_.MemoryUsage(); }

    // 用于比较内部键，使用 GetLengthPrefixedSlice 解析出切片，并将其与另一个切片进行比较
    int MemTable::KeyComparator::operator()(const char* aptr, const char* bptr) const {
        // 内部键被编码为长度前缀的字符串。
        Slice a = GetLengthPrefixedSlice(aptr);
        Slice b = GetLengthPrefixedSlice(bptr);
        return comparator.Compare(a, b);
    }

    // 用于将切片编码为内部键，并将结果存储到 scratch 字符串中
    // 它首先清空 scratch，然后将切片的长度作为变长整数编码写入 scratch、
    // 接着将切片的内容附加到 scratch 中，最后返回 scratch 的数据指针
    static const char* EncodeKey(std::string* scratch, const Slice& target) {
        scratch->clear();
        PutVarint32(scratch, target.size());
        scratch->append(target.data(), target.size());
        return scratch->data();
    }

    // MemTableIterator 类是内存表的迭代器，用于遍历内存表中的数据
    class MemTableIterator : public Iterator {
    public:
        explicit MemTableIterator(MemTable::Table* table) : iter_(table) {}

        MemTableIterator(const MemTableIterator&) = delete;

        MemTableIterator& operator=(const MemTableIterator&) = delete;

        ~MemTableIterator() override = default;

        bool Valid() const override { return iter_.Valid(); }

        void Seek(const Slice& k) override { iter_.Seek(EncodeKey(&tmp_, k)); }

        void SeekToFirst() override { iter_.SeekToFirst(); }

        void SeekToLast() override { iter_.SeekToLast(); }

        void Next() override { iter_.Next(); }

        void Prev() override { iter_.Prev(); }

        Slice key() const override { return GetLengthPrefixedSlice(iter_.key()); }

        Slice value() const override {
            Slice key_slice = GetLengthPrefixedSlice(iter_.key());
            return GetLengthPrefixedSlice(key_slice.data() + key_slice.size());
        }

        Status status() const override { return Status::OK(); }

    private:
        MemTable::Table::Iterator iter_;
        std::string tmp_;  // 用于传递给 EncodeKey
    };

    // 创建一个新的内存表迭代器，并返回其指针
    Iterator* MemTable::NewIterator() { return new MemTableIterator(&table_); }

    // 用于向内存表中添加数据，接受序列号、值类型、键和值作为参数
    // 并将它们编码后插入到内存表的数据结构中
    void MemTable::Add(SequenceNumber s, ValueType type, const Slice& key, const Slice& value) {
        //   key_size     : internal_key.size() 的 varint32
        //   key 字节     : char[internal_key.size()]
        //   tag          : uint64((sequence << 8) | type)
        //   value_size   : value.size() 的 varint32
        //   value 字节   : char[value.size()]
        size_t key_size = key.size();
        size_t val_size = value.size();
        size_t internal_key_size = key_size + 8;
        const size_t encoded_len = VarintLength(internal_key_size) +
                                    internal_key_size + VarintLength(val_size) +
                                    val_size;
        char* buf = arena_.Allocate(encoded_len);
        char* p = EncodeVarint32(buf, internal_key_size);
        std::memcpy(p, key.data(), key_size);
        p += key_size;
        EncodeFixed64(p, (s << 8) | type);
        p += 8;
        p = EncodeVarint32(p, val_size);
        std::memcpy(p, value.data(), val_size);
        assert(p + val_size == buf + encoded_len);
        table_.Insert(buf);
    }

    // 根据键查找值，首先根据给定的 LookupKey 获取内部键，然后使用内部见在内存表中查找相应的条目
    // 如果找到匹配的条目，会根据标记（tag）来确定值得类型，然后将值存储在给定的字符串中
    bool MemTable::Get(const LookupKey& key, std::string* value, Status* s) {
        Slice memkey = key.memtable_key();
        Table::Iterator iter(&table_);
        iter.Seek(memkey.data());
        if (iter.Valid()) {
            // 条目的格式是：
            //   klength  varint32
            //   userkey  char[klength]
            //   tag      uint64
            //   vlength  varint32
            //   value    char[vlength]
            // 检查它是否属于相同的用户键。我们不检查序列号，因为上面的 Seek() 调用应该已经跳过
            // 所有序列号过大的条目。
            const char* entry = iter.key();
            uint32_t key_length;
            const char* key_ptr = GetVarint32Ptr(entry, entry + 5, &key_length);
            if (comparator_.comparator.user_comparator()->Compare(
                    Slice(key_ptr, key_length - 8), key.user_key()) == 0) {
                // Correct user key
                const uint64_t tag = DecodeFixed64(key_ptr + key_length - 8);
                switch (static_cast<ValueType>(tag & 0xff)) {
                    case kTypeValue: {
                    Slice v = GetLengthPrefixedSlice(key_ptr + key_length);
                    value->assign(v.data(), v.size());
                    return true;
                    }
                    case kTypeDeletion:
                    *s = Status::NotFound(Slice());
                    return true;
                }
            }
        }
        return false;
    }

}  // namespace leveldb