#ifndef STORAGE_LEVELDB_INCLUDE_ITERATOR_H_
#define STORAGE_LEVELDB_INCLUDE_ITERATOR_H_

#include "./slice.h"
#include "./status.h"

namespace leveldb {
	// 一个封装的全局迭代器类
	class Iterator {
	public:
		Iterator();
		Iterator(const Iterator&) = delete;
		Iterator& operator=(const Iterator&) = delete;
		virtual ~Iterator();
		virtual bool Valid() const = 0; // 判断迭代器是否有效
		virtual void SeekToFirst() = 0; // 将迭代器定位到源中第一个键值对
		virtual void SeekToLast() = 0; // 将迭代器定位到源中最后一个键值对
		virtual void Seek(const Slice& target) = 0; // 将迭代器定位到大于等于目标键的第一个键值对
		virtual void Next() = 0; // 将迭代器移动到下一个键值对
		virtual void Prev() = 0; // 将迭代器移动到前一个键值对
		virtual Slice key() const = 0; // 返回当前键值对的键
		virtual Slice value() const = 0; // 返回当前键值对的值
		virtual Status status() const = 0; // 返回当前迭代器状态

		// 允许客户端注册在迭代器销毁时调用的清理函数
		using CleanupFunction = void (*)(void* arg1, void* arg2);
		void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);

	private:
		// 清理函数存储在一个单链表中。列表的头节点内联在迭代器中。
		// 用于管理迭代器销毁时的清理函数
		struct CleanupNode {
			CleanupFunction function;
			void* arg1;
			void* arg2;
			CleanupNode* next;

			bool IsEmpty() const { function == nullptr; }
			void Run() {
				assert(function != nullptr);
				(*function)(arg1, arg2);
			}
		};

		CleanupNode cleanup_head_;
	};

	// 这是两个工厂函数
	Iterator* NewEmptyIterator(); // 返回一个空的迭代器，不会产生任何数据
	Iterator* NewErrorIterator(const Status& status); // 返回一个带有指定状态的迭代器，用于表示迭代过程中出现了错误
}
#endif  // STORAGE_LEVELDB_INCLUDE_ITERATOR_H_