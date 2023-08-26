# cache
根据功能不同，leveldb 中有两种 cache
* Block cache：缓存解压后的 data block，可以加快对热数据的访问
* Table cache：缓存打开的 SSTable 文件描述符和对应的 index block、meta block
block cache 和 table cache 都是基于 **SharedLRUCache** 实现的

## SharedLRUCache
SharedLRUCache 是在 **LRUCache** 上包装了一层分片-----根据 key 的哈希值的前 4 位(kNumShardBits) 分 16个(kNumShards) LRUCache

分片的作用是减少多线程对同一个 LRUCache 对象的争用

## LRUCache
LRU 全称是 Least Recently Used(最近最少使用)，是一种根据局部性原理的缓存淘汰策略。当热点数据比较集中时，LRUCache 的效率比较高

leveldb 的 LRUCache 的实现由一个哈希表和两个链表组成：
* 链表lru_：维护 cache 中的缓存对象的使用热度。数据每次被访问，都会被插入到这个链表最新的地方。lru_->next 指向最旧的数据，lru_->prev 指向最新的数据。当 cache 占用的内存超过限制时，则从 lru_->next 开始清理数据
* 链表in_use_：维护 cache 中有哪些缓存对象被返回给调用端使用，这些数据不能被淘汰
* 哈希表 table_：保存所有 key-> 缓存对象，用于快速查找数据

## LRUHandle
LRUHandle 是 LURCache 中的一个对象
```C++
struct LRUHandle {
  void* value;
  void (*deleter)(const Slice&, void* value);
  LRUHandle* next_hash;
  LRUHandle* next;
  LRUHandle* prev;
  size_t charge;  // TODO(opt): Only allow uint32_t?
  size_t key_length;
  bool in_cache;     // Whether entry is in the cache.
  uint32_t refs;     // References, including cache reference, if present.
  uint32_t hash;     // Hash of key(); used for fast sharding and comparisons
  char key_data[1];  // Beginning of key

  Slice key() const {
    // next_ is only equal to this if the LRU handle is the list head of an
    // empty list. List heads never have meaningful keys.
    assert(next != this);

    return Slice(key_data, key_length);
  }
};
```
* next_hash：哈希表的实现采用的是**拉链法**来处理哈希冲突，用 next_hash 维护落到同一个 bucket 的对象
* next / prev：LRUCache 通过链表来维护缓存对象的使用“热度”和是否正在被调用端是哟个，这两个指针是用来实现链表的

## HandleTable
HandleTable 是 leveldb 采用拉链法实现的一个哈希表，主要提供了 Lookup、Insert、Remove 三个接口

当哈希表的元素数量超过 list_ 的长度 length_时，会调用 **Resize** 进行重新哈希。直接扫描整个哈希表进行全亮 rehash，如果哈希表很大，遇到 rehash 可能会导致系统抖动

## 缓存淘汰策略
### LRU
当热点数据比较集中时，LRU 的缓存命中率比较高。但是在某些场景下，LRU 的缓存命中率会急剧下降

leveldb 在读参数 ReadOptions 提供了一个参数 fill_cache，让上层控制是否要将 data block 放入到 block cache

### FIFO
其实就是一个队列，按先进先出的方法淘汰数据。新的数据插入队列尾部。队列满了之后从头部开始删除

### LFU
根据数据的访问频率来淘汰数据，适用于“如果数据过去被访问多次，那么将来被访问的频率也更高”的场景

### Random
随机淘汰缓存对象

### Block Cache
一个 SSTable 被打开时，会通过 options.block_cache 的 **Newld** 为其分配一个唯一的 **cache_id**

每个 data block 保存到 block cache 的 key 为 cache_id + offset

### Table Cache
table cache 的 key 是 SSTable 的 file_number，value 是一个 TableAndFile 对象

TableAndFile 有两个成员变量 file 和 table
```C++
struct TableAndFile {
    RandomAccessFile* file;
    Table* table;
};
```
table 内部封装了 index 和 filter，以及一些其他 SSTable 的元数据
```C++
struct Table::Rep {
    ~Rep() {
        delete filter;
        delete[] filter_data;
        delete index_block;
    }

    Options options;
    Status status;
    RandomAccessFile* file;
    uint64_t cache_id;
    FilterBlockReader* filter;
    const char* filter_data;

    BlockHandle metaindex_handle;  // Handle to metaindex_block: saved from footer
    Block* index_block;
};
```