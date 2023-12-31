#include "../include/comparator.h"
#include "../include/slice.h"
#include "./no_destructor.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <type_traits>

namespace leveldb {
	Comparator::~Comparator() = default;

	namespace {
		// 一个使用 字节序 来进行比较的比较器
		class BytewiseComparatorImpl : public Comparator {
		public:
			BytewiseComparatorImpl() = default;

			// 返回比较器的名称
			const char* Name() const override { return "leveldb.BytewiseComparator"; }

			// 用于比较两个切片字符串
			int Compare(const Slice& a, const Slice& b) const override {
				return a.compare(b);
			}

			void FindShortestSeparator(std::string* start, const Slice& limit) const override {
				// 查找公共前缀长度
				size_t min_length = std::min(start->size(), limit.size());
				size_t diff_index = 0;
				// 去掉公共的部分
				while ((diff_index < min_length) && ((*start)[diff_index] == limit[diff_index]))
					diff_index++;
				if (diff_index >= min_length) {
					// 此情况，一个字符串是另一个字符串的前缀，不要缩短
				}
				else {
					uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
					if (diff_byte < static_cast<uint8_t>(0xff) && diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
						(*start)[diff_index]++;
						start->resize(diff_index + 1);
						assert(Compare(*start, limit) < 0);
					}
				}
			}

			void FindShortSuccessor(std::string* key)const override {
				// 找到第一个可以递增的字符
				size_t n = key->size();
				for (size_t i = 0; i < n; i++) {
					const uint8_t byte = (*key)[i];
					if (byte != static_cast<uint8_t>(0xff)) {
						(*key)[i] = byte + 1;
						key->resize(i + 1);
						return;
					}
				}
			}
		};
	}

	const Comparator* BytewiseComparator() {
		static NoDestructor<BytewiseComparatorImpl> singleton;
		return singleton.get();
	}
}