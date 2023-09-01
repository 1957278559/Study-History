#ifndef STORAGE_LEVELDB_DB_SKIPLIST_H_
#define STORAGE_LEVELDB_DB_SKIPLIST_H_

#include <atomic>
#include <cassert>
#include <cstdlib>
#include "../util/arena.h"
#include "../util/random.h"

namespace leveldb {
	// leveldb ���߶�����
	template <typename Key, class Comparator>
	class SkipList {
	private:
		struct Node; // ���ڱ�ʾ����ĵ����ڵ�

	public:
		explicit SkipList(Comparator cmp, Arena* arena); // ����һ���µ�������Ҫ�ṩ�����ıȽ����Լ��ڴ�����ָ�����
		SkipList(const SkipList&) = delete;
		SkipList& operator=(const SkipList&) = delete;

		void Insert(const Key& key); // ������ key ���뵽�����У�ǰ���������в������� key ��ȵ���Ŀ
		bool Contains(const Key& key) const; // �ж��������Ƿ��������� key ��ȵ���Ŀ

		// �ڲ��������࣬���ڱ������������
		// �ṩ�˳�ʼ�����ж���Ч�ԡ���ȡ�ؼ��֡���ǰ�ƶ�������ƶ�����λ��ĳ��λ�õȹ���
		class Iterator {
		public:
			explicit Iterator(const SkipList* list); // ���캯������Ҫһ��Ҫ����������
			bool Valid() const; // �жϵ�ǰ������ָ��Ľڵ��Ƿ���Ч
			const Key& key() const; // ���ص�ǰ������ָ��Ľڵ�� key
			void Next(); // ��һ���ڵ�
			void Prev(); // ��һ���ڵ�
			void Seek(const Key& target); // �ҵ���һ���ڵ� key ���ڵ��� target �Ľڵ�
			void SeekToFirst(); // ָ���һ���ڵ�
			void SeekToLast(); // ָ�����һ���ڵ�

		private:
			const SkipList* list_;
			Node* node_;
		};

	private:
		enum { kMaxHeight = 12 }; // ������������߶�
		Comparator const compare_; // �洢�ȽϽڵ� key �ıȽ��������������ڲ��롢���ҵȲ����н��� key �ıȽ�
		Arena* const arena_; // ���ڸ��ڵ�����ڴ�
		Node* const head_; // �����ͷ�ڵ㣬����ʵ��������ͷ�ڵ�֮��
		std::atomic<int> max_height_; // ԭ�����ͣ����ڴ洢������������߶�
		Random rnd_; // ���������������ʵ��

		inline int GetMaxHeight() const { return max_height_.load(std::memory_order_relaxed); } // ������������߶�
		Node* NewNode(const Key& key, int height); // ���ݽڵ� key ֵ�Լ��߶ȴ���һ���½ڵ㣬�������ڴ�
		int RandomHeight(); // ����һ������Ľڵ�߶ȣ����ڹ���������
		bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); } // �Ƚ����� key �Ƿ����
		bool KeyIsAfterNode(const Key& key, Node* node) const; // �ж����� key �Ƿ���ڽڵ� node �洢������
		Node* FindGreateOrEqual(const Key& key, Node** prev) const; // ����������ڵ��ڸ��� key �Ľڵ�
		Node* FindLessThan(const Key& key) const; // ���������С�ڸ��� key �Ľڵ�
		Node* FindLast() const; // �������������һ���ڵ�
	};

	// struct Node �ڵ��ʵ��ϸ��
	template <typename Key, class Comparator>
	struct SkipList<Key, Comparator>::Node {
		explicit Node(const Key& k) : key(k) {}

		Key const key; // �洢�ڵ�� key ֵ

		// ��ȡ��ǰ�ڵ��ڵ� n �����һ���ڵ�
		// ʹ�� std::memory_order_acquire �ڴ�����м��ز�������ȷ���ڷ�����һ���ڵ�ʱ�۲쵽����һ����ȫ��ʼ���İ汾
		Node* Next(int n) {
			assert(n >= 0);
			return next_[n].load(std::memory_order_acquire);
		}

		// ���õ�ǰ�ڵ��ڵ� n �����һ���ڵ㣬��ȷ����������һ���ڵ��
		// �κ�ͨ�����ָ���ȡ�ĵط����ܹ۲쵽һ������ȫ��ʼ���Ĳ���ڵ�
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
		// ���飬�洢ָ����һ���ڵ��ָ�롣����ĳ��ȵ��ڽڵ�ĸ߶�
		// �����һ��Ԫ�� next_[0] ��ʾ����ײ��ϵ���һ���ڵ�
		// ʹ�� std::atomic ���͵�ָ�룬ȷ�����̻߳����¶Խڵ�����Ӳ������̰߳�ȫ��
		std::atomic<Node*> next_[1];
	};

	template <typename Key, class Comparator>
	typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
		char* const node_memory = arena_->AllocateAligned(sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
		return new (node_memory) Node(key);
	}

	// �ڲ�������ʵ��ϸ��
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

	// ����ʵ��ϸ��
	template <typename Key, class Comparator>
	int SkipList<Key, Comparator>::RandomHeight() {
		// �Ը��� 1 ���Ӹ߶�
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
					level--; // �л�����һ������
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
					level--; // �л�����һ������
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
					level--; // �л�����һ������
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
		//ʹ�� FindGreaterOrEqual���� �����ϰ����壬������Ϊ Insert() ���ⲿͬ���ġ�
		Node* prev[kMaxHeight];
		Node* x = FindGreateOrEqual(key, prev);

		// �������ظ�����
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