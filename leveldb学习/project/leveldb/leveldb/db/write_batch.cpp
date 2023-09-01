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
	// 存储 WriteBatch 头部大小的常量，头部包含 8字节 序列号和 4字节 的操作计数
	static const size_t kHeader = 12;

	WriteBatch::WriteBatch() { Clear(); } // 初始化一个空的 WriteBatch
	WriteBatch::~WriteBatch() = default;
	WriteBatch::Handler::~Handler() = default;

	// 清除批次中所有操作，操作计数置零并清空表示操作的数据
	void WriteBatch::Clear() {
		rep_.clear();
		rep_.resize(kHeader);
	}

	size_t WriteBatch::ApproximateSize() const { return rep_.size(); } // 返回操作数据的大小

	// 迭代处理批次中的操作
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