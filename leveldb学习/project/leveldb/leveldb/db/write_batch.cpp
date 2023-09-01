#include "../include/write_batch.h"

// WriteBatch::rep_ :=
//    sequence: fixed64
//    count: fixed32
//    data: record[count]
// record :=
//    kTypeValue varstring varstring         |
//    kTypeDeletion varstring
// varstring :=
//    len: varint32
//    data: uint8[len]

namespace leveldb {
	// �洢 WriteBatch ͷ����С�ĳ�����ͷ������ 8�ֽ� ���кź� 4�ֽ� �Ĳ�������
	static const size_t kHeader = 12;

	WriteBatch::WriteBatch() { Clear(); } // ��ʼ��һ���յ� WriteBatch
	WriteBatch::~WriteBatch() = default;
	WriteBatch::Handler::~Handler() = default;

	// ������������в����������������㲢��ձ�ʾ����������
	void WriteBatch::Clear() {
		rep_.clear();
		rep_.resize(kHeader);
	}

	size_t WriteBatch::ApproximateSize() const { return rep_.size(); } // ���ز������ݵĴ�С

	// �������������еĲ���
	Status WriteBatch::Iterate(Handler* handler) const {
		Slice input(rep_);
		if (input.size() < kHeader) {
			return Status::Corruption("malformed WriteBatch (too small)");
		}
		
		input.remove_prefix(kHeader);
		Slice key, value;
		int found = 0;
		while (!input.empty()) {
			found++;
			char tag = input[0];
			input.remove_prefix(1);
			switch (tag) {
				
			}
		}
	}


}