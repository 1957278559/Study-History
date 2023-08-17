# leveldb中一些文件的说明

## export.h
用于处理导入和导出符号的定义
主要用来支持 leveldb 的动态库功能，以便在不同平台和编译器下能正确的导入和导出函数和变量

## slice.h
一个简单的数据切片结构，包含一个指向外部存储的指针和切片的大小
用于表示一段连续的数据，例如字符串或二进制数据，但不拥有数据的所有权
用于管理数据库中的键值对数据，适用于处理不拥有数据所有权的场景
支持夸苏的数据访问和比较操作
```C++
class LEVELDB_EXPORT Slice{
private:
    const char* data_;
    size_t size_;

public:
    Slice();
    Slice(const char* d, size_t n);
    Slice(const std::string& s);
    Slice(const char* s);
    const char* data() const;
    size_t size() const;
    bool empty() const;
    char operator[](size_t n) const;
    void clear();
    void remove_prefix(size_t n);
    std::string ToString() const;
    int compare(const Slice& b) const;
    bool starts_with(const Slice& x) const;
};
//另外也重载了 == 和 != 用于比较两个切片是否相等
```