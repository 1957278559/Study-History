#include "pch.h"
#include "CppUnitTest.h"
#include <set>
#include "../LevelDB/include/status.h"
#include "../LevelDB/util/arena.h"
#include "../LevelDB/util/random.h"
#include "../LevelDB/db/skiplist.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CodeTest
{
	// 测试 status.h 文件
	TEST_CLASS(StatusTests)
	{
	public:
		// 测试移动构造
		TEST_METHOD(MoveConstructor_OK)
		{
			leveldb::Status ok = leveldb::Status::OK();
			leveldb::Status ok2 = std::move(ok);

			Assert::IsTrue(ok2.ok());
		}

		// 测试 NotFound 状态的转移和错误信息是否正确转移
		TEST_METHOD(MoveConstructor_NotFound)
		{
			leveldb::Status status = leveldb::Status::NotFound("custom NotFound status message");
			leveldb::Status status2 = std::move(status);

			Assert::IsTrue(status2.IsNotFound());
			Assert::AreEqual("NotFound: custom NotFound status message", status2.ToString().c_str());
		}

		// 测试 IOError 状态的转移
		TEST_METHOD(MoveConstructor_IOError)
		{
			leveldb::Status self_moved = leveldb::Status::IOError("custom IOError status message");

			// Needed to bypass compiler warning about explicit move-assignment.
			leveldb::Status& self_moved_reference = self_moved;
			self_moved_reference = std::move(self_moved);

			// You might want to add additional assertions here to test the behavior.
		}
	};

	// 测试 arena.h 文件
	TEST_CLASS(ArenaTests) 
	{
	public:
		// 测试一个空的 leveldb::Arena 对象，判断其能否成功初始化
		TEST_METHOD(EmptyArena)
		{
			leveldb::Arena arena;
		}

		// 简单内存分配的测试，通过循环迭代模拟随机大小的内存分配
		// 然后将特定模式的字节写入这些分配的内存中
		// 验证 Arena 的内存使用是否满足预期的增长规则，并确保之前分配的内存中的特定字节模式是正确的
		TEST_METHOD(ArenaTest_Simple)
		{
			std::vector<std::pair<size_t, char*>> allocated;
			leveldb::Arena arena;
			const int N = 100000;
			size_t bytes = 0;
			leveldb::Random rnd(301);
			for (int i = 0; i < N; i++) {
				size_t s;
				if (i % (N / 10) == 0)
					s = i;
				else
					s = rnd.OneIn(4000) ? rnd.Uniform(6000) : (rnd.OneIn(10) ? rnd.Uniform(100) : rnd.Uniform(20));
				if (s == 0)
					s = 1;
				char* r;
				if (rnd.OneIn(10))
					r = arena.AllocateAligned(s);
				else
					r = arena.Allocate(s);

				for (size_t b = 0; b < s; b++) 
					r[b] = i % 256;
				bytes += s;
				allocated.push_back(std::make_pair(s, r));
				Assert::IsTrue(arena.MemoryUsage() >= bytes);
				if (i > N / 10)
					Assert::IsTrue(arena.MemoryUsage() <= bytes * 1.10);
			}
			for (size_t i = 0; i < allocated.size(); i++) {
				size_t num_bytes = allocated[i].first;
				const char* p = allocated[i].second;
				for (size_t b = 0; b < num_bytes; b++) 
					Assert::AreEqual(static_cast<int>(i % 256), static_cast<int>(p[b]) & 0xff);
			}
		}
	};

	// 测试 skiplist.h 文件
	TEST_CLASS(SkipListTests)
	{
	public:
		typedef uint64_t Key;
		struct Comparator {
			int operator()(const Key& a, const Key& b) const {
				if (a < b) {
					return -1;
				}
				else if (a > b) {
					return +1;
				}
				else {
					return 0;
				}
			}
		};
	public:
		TEST_METHOD(EmptyTest)
		{
			leveldb::Arena arena;
			Comparator cmp;
			leveldb::SkipList<Key, Comparator> list(cmp, &arena);

			// 进行断言测试
			Assert::IsFalse(list.Contains(10));
			leveldb::SkipList<Key, Comparator>::Iterator iter(&list);
			Assert::IsFalse(iter.Valid());
			iter.SeekToFirst();
			Assert::IsFalse(iter.Valid());
			iter.Seek(100);
			Assert::IsFalse(iter.Valid());
			iter.SeekToLast();
			Assert::IsFalse(iter.Valid());
		}
		
		TEST_METHOD(InsertAndLookupTest)
		{
			const int N = 2000;
			const int R = 5000;
			leveldb::Random rnd(1000);
			std::set<Key> keys;
			leveldb::Arena arena;
			Comparator cmp;
			leveldb::SkipList<Key, Comparator> list(cmp, &arena);
			for (int i = 0; i < N; i++) {
				Key key = rnd.Next() % R;
				if (keys.insert(key).second)
					list.Insert(key);
			}

			for (int i = 0; i < R; i++)
				if (list.Contains(i))
					Assert::AreEqual((list.Contains(i) ? 1 : 0), (int)keys.count(i)); // 第一个参数是期待的值，第二个参数是实际值
			
			// Iterator 测试
			{
				leveldb::SkipList<Key, Comparator>::Iterator iter(&list);
				Assert::IsTrue(!iter.Valid());

				iter.Seek(0);
				Assert::IsTrue(iter.Valid());
				Assert::AreEqual(*(keys.begin()), iter.key());

				iter.SeekToFirst();
				Assert::IsTrue(iter.Valid());
				Assert::AreEqual(*(keys.begin()), iter.key());

				iter.SeekToLast();
				Assert::IsTrue(iter.Valid());
				Assert::AreEqual(*(keys.rbegin()), iter.key());
			}

			// Forward iteration test
			for (int i = 0; i < R; i++) {
				leveldb::SkipList<Key, Comparator>::Iterator iter(&list);
				iter.Seek(i);

				// Compare against model iterator
				std::set<Key>::iterator model_iter = keys.lower_bound(i);
				for(int j = 0; j < 3; j++) {
					if (model_iter == keys.end()) {
						Assert::IsTrue(!iter.Valid());
						break;
					}
					else {
						Assert::IsTrue(iter.Valid());
						Assert::AreEqual(*model_iter, iter.key());
						++model_iter;
						iter.Next();
					}
				}
			}

			// Backward iteration test
			{
				leveldb::SkipList<Key, Comparator>::Iterator iter(&list);
				iter.SeekToLast();

				// Compare against model iterator
				for (std::set<Key>::reverse_iterator model_iter = keys.rbegin();
					model_iter != keys.rend(); ++model_iter) {
					Assert::IsTrue(iter.Valid());
					Assert::AreEqual(*model_iter, iter.key());
					iter.Prev();
				}
				Assert::IsTrue(!iter.Valid());
			}
		}
	};
}
