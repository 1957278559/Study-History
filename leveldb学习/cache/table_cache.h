#ifndef STORAGE_LEVELDB_DB_TABLE_CACHE_H_
#define STORAGE_LEVELDB_DB_TABLE_CACHE_H_

#include <stdint.h>
#include <string>

// 这个类是线程安全的，它提供了内部同步机制，以确保在多线程环境下操作该类的方法也是安全的。

#include "db/dbformat.h"
#include "leveldb/cache.h"
#include "leveldb/table.h"
#include "port/port.h"

namespace leveldb {

    // 这个类用于管理文件系统操作，在别的地方实现
    class Env;

    class TableCache {
    public:
        // 初始化 tablecache 实例，dbname 是数据库名称，options 是数据库选项，entries 是缓存中的条目数量
        TableCache(const std::string& dbname, const Options& options, int entries);
        ~TableCache();

        // 创建一个新的迭代器，用来遍历指定文件中的表数据
        // options 是读取选项，file_number 是文件号，tableptr 一个可选参数，指向表对象的指针
        Iterator* NewIterator(const ReadOptions& options, uint64_t file_number,
                                uint64_t file_size, Table** tableptr = nullptr);

        // 获取文件中键 k 对应的值，如果找到则调用指定的处理函数
        // options 是读取选项，file_number 是文件号，file_size 是文件大小，k 是要查找的键
        // arg 是传递给处理函数的参数，handle_result 是处理函数
        Status Get(const ReadOptions& options, uint64_t file_number,
                    uint64_t file_size, const Slice& k, void* arg,
                    void (*handle_result)(void*, const Slice&, const Slice&));

        // 从缓存中移除指定文件的条目
        void Evict(uint64_t file_number);

    private:
        Status FindTable(uint64_t file_number, uint64_t file_size, Cache::Handle**);

        Env* const env_;
        const std::string dbname_;
        const Options& options_;
        Cache* cache_;
    };

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_TABLE_CACHE_H_