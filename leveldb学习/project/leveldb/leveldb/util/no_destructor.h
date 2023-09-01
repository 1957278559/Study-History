#ifndef STORAGE_LEVELDB_UTIL_NO_DESTRUCTOR_H_
#define STORAGE_LEVELDB_UTIL_NO_DESTRUCTOR_H_

#include <type_traits>
#include <utility>

namespace leveldb {
	// ��װ�ں�������̬�����е�ʵ��
	// ��ȷ��ʵ��������������Զ���ᱻ����
	// ������Ҫ�ڳ������������ڱ��ֵ�һʵ��������º�����
	template <typename InstanceType>
	class NoDestructor {
	public:
		// ���ܿɱ����ģ�壬���ڹ����װ��ʵ�����ڹ��캯���ڲ���ͨ����Ԥ������ڴ���ϵ���ԭλ���캯��������ʵ��
		template <typename... ConstructorArgTypes>
		explicit NoDestructor(ConstructorArgTypes&&... constructor_args) {
			static_assert(sizeof(instance_storage_) >= sizeof(InstanceType), "instance_storage_ is not large enough to hold the instance");
			static_assert(alignof(decltype(instance_storage_)) >= alignof(InstanceType), "instance_storage_ does not meet the instance's alignment requirement");
			new (&instance_storage_) InstanceType(std::forward<ConstructorArgTypes>(constructor_args)...);
		}

		~NoDestructor() = default;
		NoDestructor(const NoDestructor&) = delete;
		NoDestructor& operator=(const NoDestructor&) = delete;
		InstanceType* get() { return reinterpret_cast<InstanceType*>(&instance_storage_); } // ���ط�װ��ʵ����ָ�룬ͨ�����½����ڴ���е�ʵ���洢������ʵ����ָ��

	private:
		// aligned_storage �� C++ ��׼��������ʵ���ڴ����Ĺ���
		// ����һ���ڴ�飬���ڴ洢����ת��ʵ��
		// �ڴ��Ĵ�С�Ͷ��뷽ʽ��Ҫ��װ��ʵ����ƥ��
		typename std::aligned_storage<sizeof(InstanceType), alignof(InstanceType)>::type instance_storage_;
	};
}


#endif  // STORAGE_LEVELDB_UTIL_NO_DESTRUCTOR_H_