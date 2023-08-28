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
		struct Node; // ���ڱ�ʾ����ĵ����ڵ�
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
}

#endif  // STORAGE_LEVELDB_DB_SKIPLIST_H_