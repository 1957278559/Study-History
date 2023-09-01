#ifndef STORAGE_LEVELDB_INCLUDE_ITERATOR_H_
#define STORAGE_LEVELDB_INCLUDE_ITERATOR_H_

#include "./slice.h"
#include "./status.h"

namespace leveldb {
	// һ����װ��ȫ�ֵ�������
	class Iterator {
	public:
		Iterator();
		Iterator(const Iterator&) = delete;
		Iterator& operator=(const Iterator&) = delete;
		virtual ~Iterator();
		virtual bool Valid() const = 0; // �жϵ������Ƿ���Ч
		virtual void SeekToFirst() = 0; // ����������λ��Դ�е�һ����ֵ��
		virtual void SeekToLast() = 0; // ����������λ��Դ�����һ����ֵ��
		virtual void Seek(const Slice& target) = 0; // ����������λ�����ڵ���Ŀ����ĵ�һ����ֵ��
		virtual void Next() = 0; // ���������ƶ�����һ����ֵ��
		virtual void Prev() = 0; // ���������ƶ���ǰһ����ֵ��
		virtual Slice key() const = 0; // ���ص�ǰ��ֵ�Եļ�
		virtual Slice value() const = 0; // ���ص�ǰ��ֵ�Ե�ֵ
		virtual Status status() const = 0; // ���ص�ǰ������״̬

		// ����ͻ���ע���ڵ���������ʱ���õ�������
		using CleanupFunction = void (*)(void* arg1, void* arg2);
		void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);

	private:
		// �������洢��һ���������С��б��ͷ�ڵ������ڵ������С�
		// ���ڹ������������ʱ��������
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

	// ����������������
	Iterator* NewEmptyIterator(); // ����һ���յĵ���������������κ�����
	Iterator* NewErrorIterator(const Status& status); // ����һ������ָ��״̬�ĵ����������ڱ�ʾ���������г����˴���
}
#endif  // STORAGE_LEVELDB_INCLUDE_ITERATOR_H_