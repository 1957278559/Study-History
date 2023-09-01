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
	private:
		struct Node; // 用于表示跳表的单个节点

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
			explicit Iterator(const SkipList* list); // 构造函数，需要一个要遍历的跳表
			bool Valid() const; // 判断当前迭代器指向的节点是否有效
			const Key& key() const; // 返回当前迭代器指向的节点的 key
			void Next(); // 下一个节点
			void Prev(); // 上一个节点
			void Seek(const Key& target); // 找到第一个节点 key 大于等于 target 的节点
			void SeekToFirst(); // 指向第一个节点
			void SeekToLast(); // 指向最后一个节点

		private:
			const SkipList* list_;
			Node* node_;
		};

	private:
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

	// struct Node 节点的实现细节
	template <typename Key, class Comparator>
	struct SkipList<Key, Comparator>::Node {
		explicit Node(const Key& k) : key(k) {}

		Key const key; // 存储节点的 key 值

		// 获取当前节点在第 n 层的下一个节点
		// 使用 std::memory_order_acquire 内存序进行加载操作，以确保在返回下一个节点时观察到的是一个完全初始化的版本
		Node* Next(int n) {
			assert(n >= 0);
			return next_[n].load(std::memory_order_acquire);
		}

		// 设置当前节点在第 n 层的下一个节点，以确保在设置下一个节点后
		// 任何通过这个指针读取的地方都能观察到一个而完全初始化的插入节点
		void SetNext(int n, Node* x) {
			assert(n >= 0);
			next_[n].store(x, std::memory_order_release);
		}

		Node* NoBarrier_Next(int n) {
			assert(n >= 0);
			return next_[n].load(std::memory_order_relaxed);
		}

		void NoBarrier_SetNext(int n, Node* x) {
			assert(n >= 0);
			next_[n].store(x, std::memory_order_relaxed);
		}

	private:
		// 数组，存储指向下一个节点的指针。数组的长度等于节点的高度
		// 数组第一个元素 next_[0] 表示在最底层上的下一个节点
		// 使用 std::atomic 类型的指针，确保多线程环境下对节点的链接操作是线程安全的
		std::atomic<Node*> next_[1];
	};

	template <typename Key, class Comparator>
	typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
		char* const node_memory = arena_->AllocateAligned(sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
		return new (node_memory) Node(key);
	}

	// 内部迭代器实现细节
	template <typename Key, class Comparator>
	inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
		list_ = list;
		node_ = nullptr;
	}

	template <typename Key, class Comparator>
	inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
		return node_ != nullptr;
	}

	template <typename Key, class Comparator>
	inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
		assert(Valid());
		return node_->key;
	}

	template <typename Key, class Comparator>
	inline void SkipList<Key, Comparator>::Iterator::Next() {
		assert(Valid());
		node_ = node_->Next(0);
	}

	template <typename Key, class Comparator>
	inline void SkipList<Key, Comparator>::Iterator::Prev() {
		assert(Valid());
		node_ = list_->FindLessThan(node_->key);
	}

	template <typename Key, class Comparator>
	inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
		node_ = list_->FindGreateOrEqual(target, nullptr);
	}

	template <typename Key, class Comparator>
	inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
		node_ = list_->head_->Next(0);
	}

	template <typename Key, class Comparator>
	inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
		node_ = list_->FindLast();
		if (node_ == list_->head_)
			node_ = nullptr;
	}

	// 跳表实现细节
	template <typename Key, class Comparator>
	int SkipList<Key, Comparator>::RandomHeight() {
		// 以概率 1 增加高度
		static const unsigned int kBranching = 4;
		int height = 1;
		while (height < kMaxHeight && rnd_.OneIn(kBranching))
			height++;
		assert(height > 0);
		assert(height <= kMaxHeight);
		return height;
	}

	template <typename Key, class Comparator>
	bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* n) const {
		return (n != nullptr) && (compare_(n->key, key) < 0);
	}

	template <typename Key, class Comparator>
	typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindGreateOrEqual(const Key& key, Node** prev) const {
		Node* x = head_;
		int level = GetMaxHeight() - 1;
		while (true) {
			Node* next = x->Next(level);
			if (KeyIsAfterNode(key, next)) {
				x = next;
			}
			else {
				if (prev != nullptr) prev[level] = x;
				if (level == 0)
					return next;
				else
					level--; // 切换到下一层链表
			}
		}
	}

	template <typename Key, class Comparator>
	typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
		Node* x = head_;
		int level = GetMaxHeight() - 1;
		while (true) {
			assert(x == head_ || compare_(x->key, key) < 0);
			Node* next = x->Next(level);
			if (next == nullptr || compare_(next->key, key) >= 0) {
				if (level == 0)
					return x;
				else
					level--; // 切换到下一层链表
			}
			else {
				x = next;
			}
		}
	}

	template <typename Key, class Comparator>
	typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
		Node* x = head_;
		int level = GetMaxHeight() - 1;
		while (true) {
			Node* next = x->Next(level);
			if (next == nullptr) {
				if (level == 0)
					return x;
				else
					level--; // 切换到下一层链表
			}
			else {
				x = next;
			}
		}
	}

	template <typename Key, class Comparator>
	SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena) :
		compare_(cmp), arena_(arena), head_(NewNode(0, kMaxHeight)), max_height_(1), rnd_(0xdeadbeef) {
		for (int i = 0; i < kMaxHeight; i++)
			head_->SetNext(i, nullptr);
	}

	template <typename Key, class Comparator>
	void SkipList<Key, Comparator>::Insert(const Key& key) {
		//使用 FindGreaterOrEqual（） 的无障碍变体，这里因为 Insert() 是外部同步的。
		Node* prev[kMaxHeight];
		Node* x = FindGreateOrEqual(key, prev);

		// 不允许重复插入
		assert(x == nullptr || !Equal(key, x->key));

		int height = RandomHeight();
		if (height > GetMaxHeight()) {
			for (int i = GetMaxHeight(); i < height; i++)
				prev[i] = head_;
			max_height_.store(height, std::memory_order_relaxed);
		}
		x = NewNode(key, height);
		for (int i = 0; i < height; i++) {
			x->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
			prev[i]->SetNext(i, x);
		}
	}

	template <typename Key, class Comparator>
	bool SkipList<Key, Comparator>::Contains(const Key& key) const {
		Node* x = FindGreateOrEqual(key, nullptr);
		if (x != nullptr && Equal(key, x->key))
			return true;
		else
			return false;
	}
}
#endif  // STORAGE_LEVELDB_DB_SKIPLIST_H_