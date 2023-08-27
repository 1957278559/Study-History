# compaction
leveldb 的增删改通过追加写来实现，所以需要通过后台线程的 compaction 来：
* 清理过期（旧版本或者已删除）的数据
* 维护数据的有序性

## compaction 的触发
leveldb 会有集中情况自动触发 compaction
1. 当 MemTable 的大小达到阈值时，进行 MemTable 切换，然后需要将 Immutable MemTabel 刷到外村上-----称之为 Minor Compaction
2. 当 level-n 的 SSTable 超过限制，level-n 和 level-n+1 的 SSTable 会进行 conpaction-----称之为 Major Compaction
    * level-0 是通过 SSTable 的数量来判断是否需要 compaction
    * level-n(n >0) 是通过 SSTable 大小来判断是否需要 compaction

## Minor Compaction
比较简单，代码基本路径是 DBImpl::CompactMemTable => DBImpl::WriteLevel0Table => BuildTable

## Major Compaction
1. 每次 compaction 结束，更新 manifest 之后，都会调用 VersionSet::Finalize 计算下一次要进行 major compaction 的 level
2. 每次 major compaction 开始时，调用 VersionSet::PickCompaction 计算需要进行 compaction 的 SSTable
3. 如果选中的 level-n 的 SSTable 和 level-n+1 的 SSTable 的 key 的范围没有重叠，可以直接将 level-n 的 SSTable “移动” 到 level-n+1，只需要修改 Manifest
4. 否则，调用 DBImpl::DoCompactionWork 对 level-n 和 level-n+1 的 SSTable 进行多路归并

## Compaction 的问题
Compaction 会对 leveldb 的性能和稳定性带来一定的影响
1. 消耗 CPU：对 SSTable 进行解析、解压、压缩
2. 消耗 I/O：大量的 SSTable 批量读写，十几倍甚至几十倍的写放大会消耗不少 I/O，同时缩短 SSD 的寿(SSD 写入次数有限)
3. 缓存失效：删除旧的 SSTabel，生成新的 SSTabel。新 SSTable 的首次请求无法命中缓存，可能引发系统性能抖动

常见的做法是，控制 compaction 的速度（比如 RocksDB 的 Rate Limiter），让 compaction 的过程尽可能平缓，不要引起 CPU、I/O、缓存失效的毛刺。 这种做法带来一个问题：compaction 的速度应该控制在多少？Compaction 的速度如果太快，会影响系统性能；Compaction 的速度如果太慢，会阻塞写请求。 这个速度和具体的硬件能力、工作负载高度相关，往往只能设置一个“经验值”，比较难通用。同时这种做法只能在一定程度上减少系统毛刺、抖动，Compaction 带来的写放大依然是那么大。

## 写放大分析
* +1 - WAL 的写入
* +1 - Immutable Memtable 写入到 level-0 文件
* +2 - level-0 和 level-1 的 compaction
* +11 - level-n 和 level-n+1 合并的写入

所以，总的写放大是 4 + 11(n-1) = 11n -7 倍
假设有 5 个 level，写放大最大是 48 倍，也就是说，外部写入 1GB 的数据，内部观察到的 I/O 写流量会有 48GB