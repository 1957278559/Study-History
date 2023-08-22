#ifndef STORAGE_LEVELDB_DB_SKIPLIST_H_
#define STORAGE_LEVELDB_DB_SKIPLIST_H_

// 线程安全性
// 写操作需要外部同步，通常是互斥锁
// 读操作需要保证在读取过程中跳跃表不会被销毁。除此之外，读操作在没有内部锁定或同步的情况下进行

// 不变量：
// 1. 分配的节点在跳跃表被销毁之前永不被删除。这通过代码很容易保证，因为我们从不删除任何跳跃表节点。
// 2. 节点的内容（除了 next/prev 指针之外的部分）在节点链接到跳跃表之后是不可变的。
//只有 Insert() 修改列表，它会小心地初始化一个节点，并使用 release-stores 来在一个或多个列表中发布节点。

#include <atomic>
#include <cassert>
#include <cstdlib>

#include "util/arena.h"
#include "util/random.h"

namespace leveldb {

    class Arena;

    template <typename Key, class Comparator>
    class SkipList {
    private:
        struct Node;

    public:
        // 创建 SkipList 对象，并使用 cmp 来比较键
        // 并将使用 "*arena" 分配内存。在 arena 中分配的对象必须在 skiplist 对象的生命周期内保持分配。
        explicit SkipList(Comparator cmp, Arena* arena);

        SkipList(const SkipList&) = delete;
        SkipList& operator=(const SkipList&) = delete;

        // 将键插入到链表中
        void Insert(const Key& key);

        // 判断当前键是否在链表中
        bool Contains(const Key& key) const;

    // 用来遍历链表的迭代器
    class Iterator {
    public:
            // 初始化一个遍历指定列表的迭代器。
            explicit Iterator(const SkipList* list);

            // 判断迭代器是否处于有效节点上
            bool Valid() const;

            // 返回当前位置的键。
            const Key& key() const;

            // 前进到下一个位置。
            void Next();

            // 前进到上一个位置。
            void Prev();

            // 前进到第一个键大于等于目标键的位置。
            void Seek(const Key& target);

            // 定位到列表中的第一个条目。
            void SeekToFirst();

            // 定位到列表中的最后一个条目。
            void SeekToLast();

    private:
            const SkipList* list_;
            Node* node_;
    };

    private:
        enum { kMaxHeight = 12 };

        inline int GetMaxHeight() const { return max_height_.load(std::memory_order_relaxed); }

        Node* NewNode(const Key& key, int height);

        int RandomHeight();
        
        bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

        // 如果键大于节点中存储的数据，则返回 true
        bool KeyIsAfterNode(const Key& key, Node* n) const;

        // 返回最早的大于等于键的节点。如果没有这样的节点，则返回 nullptr。
        // 如果 prev 非空，则填充 prev[level] 为每个 level 中 [0..max_height_-1] 的前一个节点的指针。
        Node* FindGreaterOrEqual(const Key& key, Node** prev) const;

        // 返回最后一个键小于 key 的节点。如果没有这样的节点，则返回 head_。
        Node* FindLessThan(const Key& key) const;

        // 返回列表中的最后一个节点。如果列表为空，则返回 head_。
        Node* FindLast() const;

        Comparator const compare_;  // 用于比较
        Arena* const arena_;  // 用于分配节点的内存
        Node* const head_;  // 链表节点

        // 仅由 Insert() 修改。被读取者读取，但陈旧的值是可以的。
        std::atomic<int> max_height_;  // 链表的高度

        // Insert() 操作才写/读。rnd_ 随机数生成器。
        Random rnd_;
    };

    template <typename Key, class Comparator>
    struct SkipList<Key, Comparator>::Node {
        explicit Node(const Key& k) : key(k) {}

        Key const key;

        // 链接的访问器/修改器。包装在方法中，以便我们可以添加适当的屏障。
        Node* Next(int n) {
            assert(n >= 0);
            // 使用 'acquire load'，以便我们观察到完全初始化的返回节点版本。
            return next_[n].load(std::memory_order_acquire);
        }

        void SetNext(int n, Node* x) {
            assert(n >= 0);
            // 使用 'release store'，以便通过此指针读取的任何人都能观察到插入的节点的完全初始化版本。
            next_[n].store(x, std::memory_order_release);
        }

        // 在少数位置中可以安全使用没有屏障的变体。
        Node* NoBarrier_Next(int n) {
            assert(n >= 0);
            return next_[n].load(std::memory_order_relaxed);
        }
        
        void NoBarrier_SetNext(int n, Node* x) {
            assert(n >= 0);
            next_[n].store(x, std::memory_order_relaxed);
        }

    private:
        // 与节点高度相等的数组。next_[0] 是最低级别的链接。
        std::atomic<Node*> next_[1];
    };

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
        char* const node_memory = arena_->AllocateAligned(sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
        return new (node_memory) Node(key);
    }

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
        // 不使用显式的 "prev" 链接，而是搜索在键之前的最后一个节点。
        assert(Valid());
        node_ = list_->FindLessThan(node_->key);
        if (node_ == list_->head_) {
            node_ = nullptr;
        }
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
        node_ = list_->FindGreaterOrEqual(target, nullptr);
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
        node_ = list_->head_->Next(0);
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
        node_ = list_->FindLast();
        if (node_ == list_->head_) {
            node_ = nullptr;
        }
    }

    template <typename Key, class Comparator>
    int SkipList<Key, Comparator>::RandomHeight() {
        // 以概率 1/kBranching 增加高度
        static const unsigned int kBranching = 4;
        int height = 1;
        while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0)) {
            height++;
        }
        assert(height > 0);
        assert(height <= kMaxHeight);
        return height;
    }

    template <typename Key, class Comparator>
    bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* n) const {
        // 空的 n 被视为无限大
        return (n != nullptr) && (compare_(n->key, key) < 0);
    }

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key, Node** prev) const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            Node* next = x->Next(level);
            if (KeyIsAfterNode(key, next)) {
            // 在这个列表中继续搜索
                x = next;
            } else {
                if (prev != nullptr) prev[level] = x;
                if (level == 0) {
                    return next;
                } else {
                    // 切换到下一个列表
                    level--;
                }
            }
        }
    }

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            assert(x == head_ || compare_(x->key, key) < 0);
            Node* next = x->Next(level);
            if (next == nullptr || compare_(next->key, key) >= 0) {
                if (level == 0) {
                    return x;
                } else {
                    // 切换到下一个列表
                    level--;
                }
            } else {
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
                if (level == 0) {
                    return x;
                } else {
                    // 切换到下一个列表
                    level--;
                }
            } else {
                x = next;
            }
        }
    }

    template <typename Key, class Comparator>
    SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena)
        : compare_(cmp),
        arena_(arena),
        head_(NewNode(0 /* 任何键都可以 */, kMaxHeight)),
        max_height_(1),
        rnd_(0xdeadbeef) {
        for (int i = 0; i < kMaxHeight; i++) {
            head_->SetNext(i, nullptr);
        }
    }

    template <typename Key, class Comparator>
    void SkipList<Key, Comparator>::Insert(const Key& key) {
        // TODO(opt): 我们可以在这里使用一个无屏障的 FindGreaterOrEqual() 变体，因为 Insert() 在外部同步时是线程安全的。
        Node* prev[kMaxHeight];
        Node* x = FindGreaterOrEqual(key, prev);

        // 我们的数据结构不允许重复插入
        assert(x == nullptr || !Equal(key, x->key));

        int height = RandomHeight();
        if (height > GetMaxHeight()) {
            for (int i = GetMaxHeight(); i < height; i++) {
                prev[i] = head_;
            }
            // 可以不需要同步即可改变 max_height_，并且并发读取者在观察到 max_height_ 的新值时，
            // 要么从 head_（nullptr）读取旧值，要么在下面的循环中看到新值。
            max_height_.store(height, std::memory_order_relaxed);
        }

        x = NewNode(key, height);
        for (int i = 0; i < height; i++) {
            // NoBarrier_SetNext() 足够，因为我们在 prev[i] 中发布指向 "x" 的指针时会添加一个屏障。
            x->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
            prev[i]->SetNext(i, x);
        }
    }

    template <typename Key, class Comparator>
    bool SkipList<Key, Comparator>::Contains(const Key& key) const {
        Node* x = FindGreaterOrEqual(key, nullptr);
        if (x != nullptr && Equal(key, x->key)) {
            return true;
        } else {
            return false;
        }
    }

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_SKIPLIST_H_