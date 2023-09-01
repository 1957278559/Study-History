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

	// ���ڱ�ʾ���ݿ��ѹ������
	enum CompressionType {
		kNoCompression = 0x0, // ��ѹ��
		kSnappyCompression = 0x1, // Snappy ѹ��
		kZstdCompression = 0x2, // Zstd ѹ��
	};

	// ���ڿ������ݿ����Ϊ
	struct Options {
		Options();
		const Comparator* comparator; // �����˼���˳��ıȽ�����Ĭ��ʹ���ֵ�˳��Ƚ���
		bool create_if_missing = false; // ���Ϊ true�����ݿⲻ����ʱ�ᴴ�����ݿ�
		bool error_if_exists = false; // ���Ϊ true�����ݿ����ʱ�ᱨ��
		bool paranoid_checks = false; // ���Ϊ true����������ݴ�����ϸ��飬���ڼ�����
		Env* env; // �����뻷�������Ķ��������д�ļ�
		Logger* info_log = nullptr; // ���ڼ�¼���ݿ��ڲ����Ⱥʹ�����Ϣ����־��¼��
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

	// ���ڿ��ƶ�ȡ��������Ϊ
	// �����Ƿ�У�����ݵ�У��͡��Ƿ��ڻ�����������ݵ�
	struct ReadOptions {
		bool verify_checksums = false;
		bool fill_cache = true;
		const Snapshot* snapshot = nullptr;
	};

	// ���ڿ���д���������Ϊ�������Ƿ���Ҫͬ��д�롢�Ƿ���Ҫˢ�»����
	struct WriteOptions {
		WriteOptions() = default;
		bool sync = false;
	};
}


#endif  // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_