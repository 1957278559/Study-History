#ifndef STORAGE_LEVELDB_UTIL_NO_DESTRUCTOR_H_
#define STORAGE_LEVELDB_UTIL_NO_DESTRUCTOR_H_

#include <type_traits>
#include <utility>

namespace leveldb {
	// 封装在函数级别静态变量中的实例
	// 以确保实例的析构函数永远不会被调用
	// 这在需要在程序生命周期内保持单一实例的情况下很有用
	template <typename InstanceType>
	class NoDestructor {
	public:
		// 接受可变参数模板，用于构造封装的实例，在构造函数内部，通过在预分配的内存块上调用原位构造函数来创建实例
		template <typename... ConstructorArgTypes>
		explicit NoDestructor(ConstructorArgTypes&&... constructor_args) {
			static_assert(sizeof(instance_storage_) >= sizeof(InstanceType), "instance_storage_ is not large enough to hold the instance");
			static_assert(alignof(decltype(instance_storage_)) >= alignof(InstanceType), "instance_storage_ does not meet the instance's alignment requirement");
			new (&instance_storage_) InstanceType(std::forward<ConstructorArgTypes>(constructor_args)...);
		}

		~NoDestructor() = default;
		NoDestructor(const NoDestructor&) = delete;
		NoDestructor& operator=(const NoDestructor&) = delete;
		InstanceType* get() { return reinterpret_cast<InstanceType*>(&instance_storage_); } // 返回封装的实例的指针，通过重新解释内存块中的实例存储来返回实例的指针

	private:
		// aligned_storage 是 C++ 标准库中用来实现内存对齐的工具
		// 这是一个内存块，用于存储被封转的实例
		// 内存块的大小和对齐方式与要封装的实例相匹配
		typename std::aligned_storage<sizeof(InstanceType), alignof(InstanceType)>::type instance_storage_;
	};
}


#endif  // STORAGE_LEVELDB_UTIL_NO_DESTRUCTOR_H_