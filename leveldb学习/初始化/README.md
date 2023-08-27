# 初始化
一个 leveldb 实例初始化主要任务包括
1. 从 Manifest 文件恢复各个 level 的 SSTable 的元数据
2. 根据 log 文件恢复 MemTable
3. 恢复 last_sequence_、next_file_number_ 等信息

## DB::Open
一个 leveldb 实例的初始化从 **DB::Open** 函数开始
> Status DB::Open(const Options& options, const std::string& dbname, DB** dbptr);

* options：打开/创建 leveldb 实例的配置参数
* dbname：保存数据的目录名
* dbptr：初始化成功的 leveldb 实例保存在 *dbptr 中

DB::Open 的执行逻辑
1. 创建 DBImpl 对象：DBImpl 的构造函数会做一些简单的初始化工作
2. 调用 DBImpl::Recover
3. 根据条件决定是否需要创建新的 MemTable
4. 根据条件决定是否需要保存 Manifest
5. 删除过期文件，调度后台的 compaction 任务

## DBImpl::Recover
DBImpl::Recover 的初始化逻辑主要是：
1. 根据参数判断是否需要创建新的数据库表
2. 从 Manifest 文件恢复各个 level 的 SSTable 的元数据：调用 VersionSet::Recover 读取 Manifest 的内容
3. 文件检查：外村上的文件是否和 Manifest 的内容一致；收集需要恢复的 log 文件
4. 根据 log 文件恢复 MemTable：针对每个 log 文件调用 RecoverLogFile，同时更新 next_file_number_
5. 更新 last_sequence_

