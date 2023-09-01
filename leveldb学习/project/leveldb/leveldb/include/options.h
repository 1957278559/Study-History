#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <cstddef>

namespace leveldb {
	class Cache;
	class Comparator;
	class Env;
	class FilterPolicy;
	class Logger;
	class Snapshot;

	// 用于表示数据块的压缩类型
	enum CompressionType {
		kNoCompression = 0x0, // 无压缩
		kSnappyCompression = 0x1, // Snappy 压缩
		kZstdCompression = 0x2, // Zstd 压缩
	};

	// 用于控制数据库的行为
	struct Options {
		Options();
		const Comparator* comparator; // 定义了键的顺序的比较器，默认使用字典顺序比较器
		bool create_if_missing = false; // 如果为 true，数据库不存在时会创建数据库
		bool error_if_exists = false; // 如果为 true，数据库存在时会报错
		bool paranoid_checks = false; // 如果为 true，会进行数据处理的严格检查，用于检测错误
		Env* env; // 用于与环境交互的对象，例如读写文件
		Logger* info_log = nullptr; // 用于记录数据库内部进度和错误信息的日志记录器
		size_t write_buffer_size = 4 * 1024 * 1024;
		int max_open_files = 1000;
		Cache* block_cache = nullptr;
		size_t block_size = 4 * 1024;
		int block_restart_interval = 16;
		size_t max_file_size = 2 * 1024 * 1024;
		CompressionType compression = kSnappyCompression;
		int zstd_compression_level = 1;
		bool reuse_logs = false;
		const FilterPolicy* filter_policy = nullptr;
	};

	// 用于控制读取操作的行为
	// 包括是否校验数据的校验和、是否在缓存中填充数据等
	struct ReadOptions {
		bool verify_checksums = false;
		bool fill_cache = true;
		const Snapshot* snapshot = nullptr;
	};

	// 用于控制写入操作的行为，包括是否需要同步写入、是否需要刷新缓冲等
	struct WriteOptions {
		WriteOptions() = default;
		bool sync = false;
	};
}


#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_