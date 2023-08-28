#ifndef STORAGE_LEVELDB_DB_SKIPLIST_H_
#define STORAGE_LEVELDB_DB_SKIPLIST_H_

#include <atomic>
#include <cassert>
#include <cstdlib>
#include "../util/arena.h"
#include "../util/random.h"

namespace leveldb {
	// leveldb 的线段跳表
	template <typename Key, class Comparator>
	class SkipList {
	public:
		explicit SkipList(Comparator cmp, Arena* arena); // 创建一个新的跳表，需要提供给定的比较器以及内存分配的指针对象
		SkipList(const SkipList&) = delete;
		SkipList& operator=(const SkipList&) = delete;

		void Insert(const Key& key); // 将给定 key 插入到跳表中，前提是跳表中不存在与 key 相等的条目
		bool Contains(const Key& key) const; // 判断跳表中是否存在与给定 key 相等的条目

		// 内部迭代器类，用于遍历跳表的内容
		// 提供了初始化、判断有效性、获取关键字、向前移动、向后移动、定位到某个位置等功能
		class Iterator {
		public:
			explicit Iterator(const SkipList* list);
			bool Valid() const;
			const Key& key() const;
			void Next();
			void Prev();
			void Seek(const Key& target);
			void SeekToFirst();
			void SeekToLast();

		private:
			const SkipList* list_;
			Node* node_;
		};

	private:
		struct Node; // 用于表示跳表的单个节点
		enum { kMaxHeight = 12 }; // 定义跳表的最大高度
		Comparator const compare_; // 存储比较节点 key 的比较器，将被用于在插入、查找等操作中进行 key 的比较
		Arena* const arena_; // 用于给节点分配内存
		Node* const head_; // 跳表的头节点，跳表实际数据在头节点之后
		std::atomic<int> max_height_; // 原子整型，用于存储整个跳表的最大高度
		Random rnd_; // 用于生成随机数的实例

		inline int GetMaxHeight() const { return max_height_.load(std::memory_order_relaxed); } // 返回跳表的最大高度
		Node* NewNode(const Key& key, int height); // 根据节点 key 值以及高度创建一个新节点，并分配内存
		int RandomHeight(); // 生成一个随机的节点高度，用于构建跳表层次
		bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); } // 比较两个 key 是否相等
		bool KeyIsAfterNode(const Key& key, Node* node) const; // 判定给定 key 是否大于节点 node 存储的数据
		Node* FindGreateOrEqual(const Key& key, Node** prev) const; // 查找最早大于等于给定 key 的节点
		Node* FindLessThan(const Key& key) const; // 查找最晚的小于给定 key 的节点
		Node* FindLast() const; // 查找跳表中最后一个节点
	};
}

#endif  // STORAGE_LEVELDB_DB_SKIPLIST_H_