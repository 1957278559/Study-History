#ifndef STORAGE_LEVELDB_DB_VERSION_SET_H_
#define STORAGE_LEVELDB_DB_VERSION_SET_H_

#include <map>
#include <set>
#include <vector>

#include "db/dbformat.h"
#include "db/version_edit.h"
#include "port/port.h"
#include "port/thread_annotations.h"

namespace leveldb {

namespace log {
    class Writer;
    }

    class Compaction;
    class Iterator;
    class MemTable;
    class TableBuilder;
    class TableCache;
    class Version;
    class VersionSet;
    class WritableFile;

    // 返回最小的索引i，使得files[i]->largest >= key。
    // 如果没有这样的文件，则返回files.size()。
    // 要求："files"包含一个按非重叠顺序排序的文件列表。
    int FindFile(const InternalKeyComparator& icmp, const std::vector<FileMetaData*>& files, const Slice& key);


    // 如果"files"中的某个文件与用户键范围[*smallest,*largest]重叠，
    // 则返回true。如果disjoint_sorted_files为true，则文件[]包含已排序的不相交范围。
    bool SomeFileOverlapsRange(const InternalKeyComparator& icmp,
                            bool disjoint_sorted_files,
                            const std::vector<FileMetaData*>& files,
                            const Slice* smallest_user_key,
                            const Slice* largest_user_key);

    class Version {
    public:
        // 查找键的值。如果找到，将其存储在*val中并返回OK。
        // 否则返回非OK状态。填充*stats。
        // 要求：锁未持有
        struct GetStats {
            FileMetaData* seek_file;
            int seek_file_level;
        };

        // 向*iters附加一系列迭代器，当合并在一起时，将产生此Version的内容。
        // 要求：已保存此版本（见VersionSet::SaveTo）
        void AddIterators(const ReadOptions&, std::vector<Iterator*>* iters);

        Status Get(const ReadOptions&, const LookupKey& key, std::string* val, GetStats* stats);

        // 将"stats"添加到当前状态中。如果需要触发新的压缩，则返回true，否则返回false。
        // 要求：锁已持有
        bool UpdateStats(const GetStats& stats);

        // 记录指定内部键的读取样本。大约每config::kReadBytesPeriod字节采样一次。
        // 如果需要触发新的压缩，则返回true。
        // 要求：锁已持有
        bool RecordReadSample(Slice key);

        // Reference count management (so Versions do not disappear out from
        // under live iterators)
        void Ref();
        void Unref();

        void GetOverlappingInputs(
            int level,
            const InternalKey* begin,  // nullptr表示在所有键之前
            const InternalKey* end,    // nullptr表示在所有键之后
            std::vector<FileMetaData*>* inputs);

        // 返回true，如果指定级别的某些文件与[*smallest_user_key,*largest_user_key]的某些部分重叠。
        // smallest_user_key==nullptr表示小于数据库中所有键。
        // largest_user_key==nullptr表示大于数据库中所有键。
        bool OverlapInLevel(int level, const Slice* smallest_user_key,
                            const Slice* largest_user_key);

        // 返回适合存储范围[smallest_user_key,largest_user_key]的新内存表压缩结果的级别。
        int PickLevelForMemTableOutput(const Slice& smallest_user_key,
                                        const Slice& largest_user_key);

        int NumFiles(int level) const { return files_[level].size(); }

        // Return a human readable string that describes this version's contents.
        std::string DebugString() const;

    private:
        friend class Compaction;
        friend class VersionSet;

        class LevelFileNumIterator;

        explicit Version(VersionSet* vset)
            : vset_(vset),
                next_(this),
                prev_(this),
                refs_(0),
                file_to_compact_(nullptr),
                file_to_compact_level_(-1),
                compaction_score_(-1),
                compaction_level_(-1) {}

        Version(const Version&) = delete;
        Version& operator=(const Version&) = delete;

        ~Version();

        Iterator* NewConcatenatingIterator(const ReadOptions&, int level) const;

        // Call func(arg, level, f) for every file that overlaps user_key in
        // order from newest to oldest.  If an invocation of func returns
        // false, makes no more calls.
        //
        // REQUIRES: user portion of internal_key == user_key.
        void ForEachOverlapping(Slice user_key, Slice internal_key, void* arg,
                                bool (*func)(void*, int, FileMetaData*));

        VersionSet* vset_;  // 此Version所属的VersionSet
        Version* next_;     // 链表中的下一个版本
        Version* prev_;     // 链表中的上一个版本
        int refs_;          // 对此版本的活动引用数

        // 每个级别的文件列表
        std::vector<FileMetaData*> files_[config::kNumLevels];

        // 下一个基于寻找统计信息的压缩的文件
        FileMetaData* file_to_compact_;
        int file_to_compact_level_;
 
        // 下一个应进行压缩并且其压缩得分大于1的级别，初始化由Finalize()设置。
        double compaction_score_;
        int compaction_level_;
    };

    class VersionSet {
    public:
        VersionSet(const std::string& dbname, const Options* options,
                    TableCache* table_cache, const InternalKeyComparator*);
        VersionSet(const VersionSet&) = delete;
        VersionSet& operator=(const VersionSet&) = delete;

        ~VersionSet();

        // 将*edit应用于当前版本，以形成一个新描述符，
        // 该描述符既保存到持久状态中，又安装为新的当前版本。
        // 将在实际写入文件时释放*mu。
        // 要求：*mu在进入时被持有。
        // 要求：没有其他线程同时调用LogAndApply()
        Status LogAndApply(VersionEdit* edit, port::Mutex* mu)
            EXCLUSIVE_LOCKS_REQUIRED(mu);

        // 恢复最后保存的描述符的持久存储。
        Status Recover(bool* save_manifest);

        // 返回当前版本。
        Version* current() const { return current_; }

        // 返回当前的描述符文件编号
        uint64_t ManifestFileNumber() const { return manifest_file_number_; }

        // 分配并返回一个新文件编号
        uint64_t NewFileNumber() { return next_file_number_++; }

        // 安排重用"file_number"，除非已分配更新的文件编号。
        // 要求："file_number"是通过调用NewFileNumber()返回的。
        void ReuseFileNumber(uint64_t file_number) {
            if (next_file_number_ == file_number + 1) {
            next_file_number_ = file_number;
            }
        }

        // 返回指定级别的Table文件数。
        int NumLevelFiles(int level) const;

        // 返回指定级别的所有文件的组合文件大小。
        int64_t NumLevelBytes(int level) const;

        // 返回最后一个序列号。
        uint64_t LastSequence() const { return last_sequence_; }

        // 将最后一个序列号设置为s。
        void SetLastSequence(uint64_t s) {
            assert(s >= last_sequence_);
            last_sequence_ = s;
        }

         // 将指定文件编号标记为已使用。
        void MarkFileNumberUsed(uint64_t number);

        // 返回当前日志文件编号。
        uint64_t LogNumber() const { return log_number_; }

        // 返回当前正在压缩的日志文件的日志文件编号，如果没有这样的日志文件，则返回零
        uint64_t PrevLogNumber() const { return prev_log_number_; }

        // 选择新压缩的级别和输入。
        // 如果没有需要压缩的内容，则返回nullptr。
        // 否则返回一个指向堆分配的描述压缩的对象的指针。
        // 调用者应删除结果。
        Compaction* PickCompaction();

        // 返回一个用于压缩范围[level, level+1]的迭代器。
        // 如果在该级别没有与指定范围重叠的内容，则返回nullptr。
        // 调用者应删除结果。
        Compaction* CompactRange(int level, const InternalKey* begin,
                                const InternalKey* end);

        // 返回下一级别中某个文件的最大重叠数据（字节数）。
        int64_t MaxNextLevelOverlappingBytes();

        // 创建一个迭代器，用于读取"*c"的压缩输入。
        // 调用者在不再需要时应删除迭代器。
        Iterator* MakeInputIterator(Compaction* c);

        // 如果某个级别需要压缩，则返回true。
        bool NeedsCompaction() const {
            Version* v = current_;
            return (v->compaction_score_ >= 1) || (v->file_to_compact_ != nullptr);
        }

        // 将所有活动版本中列出的文件添加到*live。
        // 可能会修改某些内部状态。
        void AddLiveFiles(std::set<uint64_t>* live);

        // 返回与"v"版本的指定键"key"的数据在数据库中的近似偏移量。
        uint64_t ApproximateOffsetOf(Version* v, const InternalKey& key);

        // 返回可读的关于每个级别文件数量的简短人类可读的摘要（单行）。
        // 使用*scratch作为支持存储。
        struct LevelSummaryStorage {
            char buffer[100];
        };
        const char* LevelSummary(LevelSummaryStorage* scratch) const;

    private:
        class Builder;

        friend class Compaction;
        friend class Version;

        bool ReuseManifest(const std::string& dscname, const std::string& dscbase);

        void Finalize(Version* v);

        void GetRange(const std::vector<FileMetaData*>& inputs, InternalKey* smallest,
                        InternalKey* largest);

        void GetRange2(const std::vector<FileMetaData*>& inputs1,
                        const std::vector<FileMetaData*>& inputs2,
                        InternalKey* smallest, InternalKey* largest);

        void SetupOtherInputs(Compaction* c);

        // Save current contents to *log
        Status WriteSnapshot(log::Writer* log);

        void AppendVersion(Version* v);

        Env* const env_;
        const std::string dbname_;
        const Options* const options_;
        TableCache* const table_cache_;
        const InternalKeyComparator icmp_;
        uint64_t next_file_number_;
        uint64_t manifest_file_number_;
        uint64_t last_sequence_;
        uint64_t log_number_;
        uint64_t prev_log_number_;  // 0 or backing store for memtable being compacted

        // Opened lazily
        WritableFile* descriptor_file_;
        log::Writer* descriptor_log_;
        Version dummy_versions_;  // Head of circular doubly-linked list of versions.
        Version* current_;        // == dummy_versions_.prev_

        // Per-level key at which the next compaction at that level should start.
        // Either an empty string, or a valid InternalKey.
        std::string compact_pointer_[config::kNumLevels];
    };

    // A Compaction encapsulates information about a compaction.
    class Compaction {
    public:
        ~Compaction();

        // Return the level that is being compacted.  Inputs from "level"
        // and "level+1" will be merged to produce a set of "level+1" files.
        int level() const { return level_; }

        // Return the object that holds the edits to the descriptor done
        // by this compaction.
        VersionEdit* edit() { return &edit_; }

        // "which" must be either 0 or 1
        int num_input_files(int which) const { return inputs_[which].size(); }

        // Return the ith input file at "level()+which" ("which" must be 0 or 1).
        FileMetaData* input(int which, int i) const { return inputs_[which][i]; }

        // Maximum size of files to build during this compaction.
        uint64_t MaxOutputFileSize() const { return max_output_file_size_; }

        // Is this a trivial compaction that can be implemented by just
        // moving a single input file to the next level (no merging or splitting)
        bool IsTrivialMove() const;

        // Add all inputs to this compaction as delete operations to *edit.
        void AddInputDeletions(VersionEdit* edit);

        // Returns true if the information we have available guarantees that
        // the compaction is producing data in "level+1" for which no data exists
        // in levels greater than "level+1".
        bool IsBaseLevelForKey(const Slice& user_key);

        // Returns true iff we should stop building the current output
        // before processing "internal_key".
        bool ShouldStopBefore(const Slice& internal_key);

        // Release the input version for the compaction, once the compaction
        // is successful.
        void ReleaseInputs();

    private:
        friend class Version;
        friend class VersionSet;

        Compaction(const Options* options, int level);

        int level_;
        uint64_t max_output_file_size_;
        Version* input_version_;
        VersionEdit edit_;

        // Each compaction reads inputs from "level_" and "level_+1"
        std::vector<FileMetaData*> inputs_[2];  // The two sets of inputs

        // State used to check for number of overlapping grandparent files
        // (parent == level_ + 1, grandparent == level_ + 2)
        std::vector<FileMetaData*> grandparents_;
        size_t grandparent_index_;  // Index in grandparent_starts_
        bool seen_key_;             // Some output key has been seen
        int64_t overlapped_bytes_;  // Bytes of overlap between current output
                                    // and grandparent files

        // State for implementing IsBaseLevelForKey

        // level_ptrs_ holds indices into input_version_->levels_: our state
        // is that we are positioned at one of the file ranges for each
        // higher level than the ones involved in this compaction (i.e. for
        // all L >= level_ + 2).
        size_t level_ptrs_[config::kNumLevels];
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_VERSION_SET_H_