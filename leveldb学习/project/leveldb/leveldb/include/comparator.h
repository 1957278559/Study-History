#ifndef STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_
#define STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_

#include <string>

namespace leveldb {
	class Slice;

	// ����Ϊ leveldb �еļ�(key)�ṩȫ�ֵıȽ�˳��ĳ���
	// ��Ҫ��������ʵ����Щ����
	class Comparator {
	public:
		virtual ~Comparator();

		// ִ����·�Ƚϣ��Ƚ����� Slice ��������ݣ������ǣ�
		// ���ظ�������� a С�� b
		// �����㣺��� a ���� b
		// ������������� a ���� b
		virtual int Compare(const Slice& a, const Slice& b) const = 0;

		// ���رȽ������Ƶ��ַ���
		virtual const char* Name() const = 0;

		// �����Ż��ڲ����ݽṹ���Լ���������(index block)�����ݽṹ�Ŀռ�Ҫ��
		// ��� *start < limit���������Խ� *start �޸�Ϊһ���� [start, limit) ��Χ�ڽ϶̵��ַ���
		virtual void FindShortestSeparator(std::string* start, const Slice& limit) const = 0;

		// �� *key �޸�Ϊһ�����ڵ��� *key �Ľ϶��ַ���
		virtual void FindShortSuccessor(std::string* key) const = 0;
	};

	// ����һ���ڽ��Ƚ��������ָ�룬���ֽ����������
	const Comparator* BytewiseComparator();
}


#endif  // STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_