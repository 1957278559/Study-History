#ifndef STORAGE_LEVELDB_INCLUDE_WRITE_BATCH_H_
#define STORAGE_LEVELDB_INCLUDE_WRITE_BATCH_H_

#include "./status.h"
#include <string>

namespace leveldb {
	class Slice;

	// ���ڱ�ʾһ�����ݿ������������һ��������һ���ύ
	class WriteBatch {
	public:
		// Ƕ���࣬���ڶ�������������Ľӿڣ�������������һ���û��ṩ���࣬���ڴ������
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

		void Put(const Slice& key, const Slice& value); // �����������һ�� key->value ��ӳ��
		void Delete(const Slice& key); // �����������һ��ɾ��������ɾ����������ӳ��
		void Clear(); // ��������е����л���ĸ��²���
		size_t ApproximateSize() const; // ���������и������ݿ�������Ĵ�С�仯�Ľ���ֵ����ֵ���� leveldb ������ָ��
		void Append(const WriteBatch& source); // ����һ�������еĲ���׷�ӵ���ǰ������
		Status Iterate(Handler* handler) const; // ���ڵ������������еĲ�����ͨ������һ��ʵ���� Handle �ӿڵĶ��󣬿��Դ���ÿ������

	private:
		friend class WriteBatchInternal;

		// ���ڴ������εĲ��������ڲ�ʵ���У�����ÿ�ָ�ʽ�洢����
		std::string rep_;
	};
}


#endif  // STORAGE_LEVELDB_INCLUDE_WRITE_BATCH_H_