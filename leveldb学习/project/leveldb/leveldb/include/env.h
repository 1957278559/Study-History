#ifndef STORAGE_LEVELDB_INCLUDE_ENV_H_
#define STORAGE_LEVELDB_INCLUDE_ENV_H_

// Env �� leveldb ʵ�����ڷ��ʲ���ϵͳ���ܵĽӿ�
// ���÷������ڴ����ݿ�ʱ�ṩ�Զ���� Env �����Ի�ø���ϸ�Ŀ���
// ���� Env ʵ�ֶ������ڶ���߳�֮����в������ʣ��������κ��ⲿͬ��

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

		static Env* Default(); // ����Ĭ�ϵ� Env ����
		virtual Status NewSequentialFile(const std::string& fname, SequentialFile** result) = 0; // ����˳���ȡָ�����Ƶ��ļ�
		virtual Status NewRandomAccessFile(const std::string& fname, RandomAccessFile** result) = 0; // ��ָ�����Ƶ��ļ�����������ʶ�ȡ
		virtual Status NewWritableFile(const std::string& fname, WritableFile** result) = 0; // ����д��һ�����ļ������ͬ���ļ����ڣ����ɾ����������һ�����ļ�
		virtual Status NewAppendableFile(const std::string& fname, WritableFile** result); // ����׷�ӵ������ļ��򴴽�һ�����ļ�

		virtual bool FileExists(const std::string& fname) = 0; // ���ָ���ļ��Ƿ����
		virtual Status GetChildren(const std::string& dir, std::vector<std::string>* result) = 0; // ��ȡָ��Ŀ¼�µ����ļ��б�

		virtual Status RemoveFile(const std::string& fname); // ɾ��ָ�����Ƶ��ļ�
		virtual Status DeleteFile(const std::string& fname);
		
		virtual Status CreateDir(const std::string& dirname) = 0; // ����ָ�����Ƶ�Ŀ¼
		virtual Status RemoveDir(const std::string& dirname); // ɾ��ָ�����Ƶ�Ŀ¼
		virtual Status DeleteDir(const std::string& dirname);

		virtual Status GetFileSize(const std::string& fname, uint64_t* file_size) = 0; // ��ȡָ���ļ���С
		virtual Status RenameFile(const std::string& src, const std::string& target) = 0; // �ļ�������

		virtual Status LockFile(const std::string& fname, FileLock** lock) = 0; // ����ָ���ļ������ڷ�ֹ�������ͬʱ����ͬһ���ݿ�
		virtual Status UnlockFile(FileLock* lock) = 0; // ����

		virtual void Schedule(void (*function)(void* arg), void* arg) = 0; // �����ں�̨�߳������к���
		virtual void StartThread(void (*function)(void* arg), void* arg) = 0; // ����һ�����̣߳������߳������к���

		virtual Status GetTestDirectory(std::string* path) = 0; // ���ڲ��Ե���ʱĿ¼

		virtual Status NewLogger(const std::string& fname, Logger** result) = 0; // ����������һ�����ڴ洢��Ϣ��Ϣ����־�ļ�

		virtual uint64_t NowMicros() = 0; // ������ĳ���̶�ʱ���������΢���������ڼ���ʱ���
		virtual void SleepForMicroseconds(int micros) = 0; // ʹ��ǰ�߳�����ָ����΢����
	};

	// һ���ӿ��࣬���ڱ�ʾ˳���ȡ�ļ�
	class SequentialFile {
	public:
		SequentialFile() = default;
		SequentialFile(const SequentialFile&) = delete;
		SequentialFile& operator=(const SequentialFile&) = delete;
		virtual ~SequentialFile() = default;

		// ���ڴ��ļ��ж�ȡ��� n �ֽڵ����ݣ����ݻᱻ��ȡ�� scratch ������������ result ����Ϊ�Ѷ�ȡ���ݵ� Slice
		// ��Ҫ�ⲿͬ��
		virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

		// �����ļ��е� n ���ֽ�
		// ��Ҫ�ⲿͬ��
		virtual Status Skip(uint64_t n) = 0;
	};

	// һ���ӿ��࣬���ڱ��������ȡ�ļ�
	class RandomAccessFile {
	public:
		RandomAccessFile() = default;
		RandomAccessFile(const RandomAccessFile&) = delete;
		RandomAccessFile& operator=(const RandomAccessFile&) = delete;
		virtual ~RandomAccessFile() = default;

		// ���ļ�ָ��ƫ��λ�� offset ��ʼ��ȡ��� n�ֽ� ������
		virtual Status Read(uint64_t offset, size_t n, Slice* result, char* scratch) const = 0;
	};

	// ֧��˳��д����ļ�������
	class WritableFile {
	public:
		WritableFile() = default;
		WritableFile(const WritableFile&) = delete;
		WritableFile& operator=(const WritableFile&) = delete;
		virtual ~WritableFile() = default;

		virtual Status Append(const Slice& data) = 0; //����׷�ӵ��ļ�ĩβ
		virtual Status Close() = 0; // �ر��ļ�
		virtual Status Flush() = 0; // ���ڲ��������е�����ˢ�µ��ļ��У�ȷ����д������ݱ��־û����ļ�ϵͳ
		virtual Status Sync() = 0; // �����л�������ͬ�������̣���ȷ�����ݳ־���
	};

	// ��־��¼�ĳ�����
	class Logger {
	public:
		Logger() = default;
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		virtual ~Logger() = default;

		// ��ָ����ʽ����־��Ŀ��¼����־�ļ��У�����һ����ʽ�ַ�����һ�������б�(std::va_list)��Ϊ����
		// ���ݸ�ʽ�ַ���������־��Ŀ������д����־�ļ�
		virtual void Logv(const char* format, std::va_list ap) = 0;
	};

	// �ļ�����
	class FileLock {
	public:
		FileLock() = default;
		FileLock(const FileLock&) = delete;
		FileLock& operator=(const FileLock&) = delete;
		virtual ~FileLock() = default;
	};

	// ���ڼ�¼��־��Ϣ
	void Log(Logger* info_log, const char* format, ...)
#if defined(__GNUC__) || defined(__clang__)
		__attribute__((__format__(__printf__, 2, 3)))
#endif
		;

	Status WriteStringToFile(Env* env, const Slice& data, const std::string& fname);
	Status ReadFileToString(Env* env, const std::string& fname, std::string* data);

	// Env ���һ����װ������Ҫ�ǵ���ת��
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