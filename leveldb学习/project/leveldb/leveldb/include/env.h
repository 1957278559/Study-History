#ifndef STORAGE_LEVELDB_INCLUDE_ENV_H_
#define STORAGE_LEVELDB_INCLUDE_ENV_H_

// Env 是 leveldb 实现用于访问操作系统功能的接口
// 调用方可以在打开数据库时提供自定义的 Env 对象，以获得更精细的控制
// 所有 Env 实现都可以在多个线程之间进行并发访问，而无需任何外部同步

#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>
#include "./status.h"

namespace leveldb {
	class FileLock;
	class Logger;
	class RandomAccessFile;
	class SequentialFile;
	class Slice;
	class WritableFile;
	  
	class Env {
	public:
		Env() = default;
		Env(const Env&) = delete;
		Env& operator=(const Env&) = delete;
		virtual ~Env() = default;

		static Env* Default(); // 返回默认的 Env 对象
		virtual Status NewSequentialFile(const std::string& fname, SequentialFile** result) = 0; // 用于顺序读取指定名称的文件
		virtual Status NewRandomAccessFile(const std::string& fname, RandomAccessFile** result) = 0; // 从指定名称的文件进行随机访问读取
		virtual Status NewWritableFile(const std::string& fname, WritableFile** result) = 0; // 用于写入一个新文件，如果同名文件存在，则会删除它并创建一个新文件
		virtual Status NewAppendableFile(const std::string& fname, WritableFile** result); // 用于追加到现有文件或创建一个新文件

		virtual bool FileExists(const std::string& fname) = 0; // 检查指定文件是否存在
		virtual Status GetChildren(const std::string& dir, std::vector<std::string>* result) = 0; // 获取指定目录下的子文件列表

		virtual Status RemoveFile(const std::string& fname); // 删除指定名称的文件
		virtual Status DeleteFile(const std::string& fname);
		
		virtual Status CreateDir(const std::string& dirname) = 0; // 创建指定名称的目录
		virtual Status RemoveDir(const std::string& dirname); // 删除指定名称的目录
		virtual Status DeleteDir(const std::string& dirname);

		virtual Status GetFileSize(const std::string& fname, uint64_t* file_size) = 0; // 获取指定文件大小
		virtual Status RenameFile(const std::string& src, const std::string& target) = 0; // 文件重命名

		virtual Status LockFile(const std::string& fname, FileLock** lock) = 0; // 锁定指定文件，用于防止多个进程同时访问同一数据库
		virtual Status UnlockFile(FileLock* lock) = 0; // 解锁

		virtual void Schedule(void (*function)(void* arg), void* arg) = 0; // 安排在后台线程中运行函数
		virtual void StartThread(void (*function)(void* arg), void* arg) = 0; // 启动一个新线程，并在线程中运行函数

		virtual Status GetTestDirectory(std::string* path) = 0; // 用于测试的临时目录

		virtual Status NewLogger(const std::string& fname, Logger** result) = 0; // 创建并返回一个用于存储信息消息的日志文件

		virtual uint64_t NowMicros() = 0; // 返回自某个固定时间点依赖的微秒数，用于计算时间差
		virtual void SleepForMicroseconds(int micros) = 0; // 使当前线程休眠指定的微秒数
	};

	// 一个接口类，用于表示顺序读取文件
	class SequentialFile {
	public:
		SequentialFile() = default;
		SequentialFile(const SequentialFile&) = delete;
		SequentialFile& operator=(const SequentialFile&) = delete;
		virtual ~SequentialFile() = default;

		// 用于从文件中读取最多 n 字节的数据，数据会被读取到 scratch 缓冲区，并将 result 设置为已读取数据的 Slice
		// 需要外部同步
		virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

		// 跳过文件中的 n 个字节
		// 需要外部同步
		virtual Status Skip(uint64_t n) = 0;
	};

	// 一个接口类，用于表四随机读取文件
	class RandomAccessFile {
	public:
		RandomAccessFile() = default;
		RandomAccessFile(const RandomAccessFile&) = delete;
		RandomAccessFile& operator=(const RandomAccessFile&) = delete;
		virtual ~RandomAccessFile() = default;

		// 从文件指定偏移位置 offset 开始读取最多 n字节 的数据
		virtual Status Read(uint64_t offset, size_t n, Slice* result, char* scratch) const = 0;
	};

	// 支持顺序写入的文件抽象类
	class WritableFile {
	public:
		WritableFile() = default;
		WritableFile(const WritableFile&) = delete;
		WritableFile& operator=(const WritableFile&) = delete;
		virtual ~WritableFile() = default;

		virtual Status Append(const Slice& data) = 0; //数据追加到文件末尾
		virtual Status Close() = 0; // 关闭文件
		virtual Status Flush() = 0; // 将内部缓冲区中的数据刷新到文件中，确保已写入的数据被持久化到文件系统
		virtual Status Sync() = 0; // 将所有缓冲数据同步到磁盘，以确保数据持久性
	};

	// 日志记录的抽象类
	class Logger {
	public:
		Logger() = default;
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		virtual ~Logger() = default;

		// 以指定格式将日志条目记录到日志文件中，采用一个格式字符串和一个参数列表(std::va_list)作为输入
		// 根据格式字符串生成日志条目并将其写入日志文件
		virtual void Logv(const char* format, std::va_list ap) = 0;
	};

	// 文件锁类
	class FileLock {
	public:
		FileLock() = default;
		FileLock(const FileLock&) = delete;
		FileLock& operator=(const FileLock&) = delete;
		virtual ~FileLock() = default;
	};

	// 用于记录日志信息
	void Log(Logger* info_log, const char* format, ...)
#if defined(__GNUC__) || defined(__clang__)
		__attribute__((__format__(__printf__, 2, 3)))
#endif
		;

	Status WriteStringToFile(Env* env, const Slice& data, const std::string& fname);
	Status ReadFileToString(Env* env, const std::string& fname, std::string* data);

	// Env 类的一个包装器，主要是调用转发
	class EnvWrapper : public Env {
	public:
		explicit EnvWrapper(Env* t) : target_(t) {}
		virtual ~EnvWrapper() {}

		Env* target() const { return target_; }
		Status NewSequentialFile(const std::string& f, SequentialFile** r) override { return target_->NewSequentialFile(f, r); }
		Status NewRandomAccessFile(const std::string& f, RandomAccessFile** r) override { return target_->NewRandomAccessFile(f, r); }
		Status NewWritableFile(const std::string& f, WritableFile** r) override { return target_->NewWritableFile(f, r); }
		Status NewAppendableFile(const std::string& f, WritableFile** r) override { return target_->NewAppendableFile(f, r); }
		bool FileExists(const std::string& f) override { return target_->FileExists(f); }
		Status GetChildren(const std::string& dir, std::vector<std::string>* r) override { return target_->GetChildren(dir, r); }
		Status RemoveFile(const std::string& f) override { return target_->RemoveFile(f); }
		Status CreateDir(const std::string& d) override { return target_->CreateDir(d); }
		Status RemoveDir(const std::string& d) override { return target_->RemoveDir(d); }
		Status GetFileSize(const std::string& f, uint64_t* s) override { return target_->GetFileSize(f, s); }
		Status RenameFile(const std::string& s, const std::string& t) override { return target_->RenameFile(s, t); }
		Status LockFile(const std::string& f, FileLock** l) override { return target_->LockFile(f, l); }
		Status UnlockFile(FileLock* l) override { return target_->UnlockFile(l); }
		void Schedule(void (*f)(void*), void* a) override { return target_->Schedule(f, a); }
		void StartThread(void (*f)(void*), void* a) override { return target_->StartThread(f, a); }
		Status GetTestDirectory(std::string* path) override { return target_->GetTestDirectory(path); }
		Status NewLogger(const std::string& fname, Logger** result) override { return target_->NewLogger(fname, result); }
		uint64_t NowMicros() override { return target_->NowMicros(); }
		void SleepForMicroseconds(int micros) override { target_->SleepForMicroseconds(micros); }

	private:
		Env* target_;
	};
}

#endif  // STORAGE_LEVELDB_INCLUDE_ENV_H_