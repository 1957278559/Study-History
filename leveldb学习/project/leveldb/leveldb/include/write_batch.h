#ifndef STORAGE_LEVELDB_INCLUDE_WRITE_BATCH_H_
#define STORAGE_LEVELDB_INCLUDE_WRITE_BATCH_H_

#include "./status.h"
#include <string>

namespace leveldb {
	class Slice;

	// 用于表示一批数据库操作，可以在一个事务中一起提交
	class WriteBatch {
	public:
		// 嵌套类，用于定义操作处理器的接口，操作处理器是一个用户提供的类，用于处理操作
		class Handler {
		public:
			virtual ~Handler();
			virtual void Put(const Slice& key, const Slice& value) = 0;
			virtual void Delete(const Slice& key) = 0;
		};

		WriteBatch();
		WriteBatch(const WriteBatch&) = default;
		WriteBatch& operator=(const WriteBatch&) = default;
		~WriteBatch();

		void Put(const Slice& key, const Slice& value); // 在批次中添加一个 key->value 的映射
		void Delete(const Slice& key); // 在批次中添加一个删除操作，删除给定键的映射
		void Clear(); // 清除批次中的所有缓冲的更新操作
		size_t ApproximateSize() const; // 返回批次中更新数据库所引起的大小变化的近似值，该值用于 leveldb 的性能指标
		void Append(const WriteBatch& source); // 将另一个批次中的操作追加到当前批次中
		Status Iterate(Handler* handler) const; // 用于迭代处理批次中的操作，通过传递一个实现了 Handle 接口的对象，可以处理每个操作

	private:
		friend class WriteBatchInternal;

		// 用于储存批次的操作，在内部实现中，他以每种格式存储操作
		std::string rep_;
	};
}


#endif  // STORAGE_LEVELDB_INCLUDE_WRITE_BATCH_H_