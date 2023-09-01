#ifndef STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_
#define STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_

#include <string>

namespace leveldb {
	class Slice;

	// ���Բ��ԵĶ����ʵ��
	// ���˲������ڴ���һ��С�͹��������Դ�һ��������ɹ�����
	// ��Щ�������洢�� leveldb �У������� leveldb ���Զ����ݹ������������Ƿ�Ӵ��̶�ȡĳЩ��Ϣ
	// ͨ������£����������Խ����̲��ҵ������Ӷ�μ��ٵ�ÿ�� DB::Get() ����ֻ��һ�δ��̲���
	class FilterPolicy {
	public:
		virtual ~FilterPolicy() {}
		virtual const char* Name() const = 0; // ���麯�������ڻ�ȡ���Բ��Ե�����
		virtual void CreateFilter(const Slice* keys, int n, std::string* dst) const = 0; // ����һ�������һ��������
		virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0; // �ж�ĳ�����Ƿ����ƥ�������
	};

	//const FilterPolicy* NewBloomFilterPolicy(int bits_per_key); // ���ڴ���һ������ Bloom ���������¹��˲��ԣ���������һ��ָ�� FilterPolicy ��ָ��
}
#endif  // STORAGE_LEVELDB_INCLUDE_FILTER_POLICY_H_