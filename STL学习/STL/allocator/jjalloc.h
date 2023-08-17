#ifndef _JJALLOC_
#define _JJALLOC_

#include <new>			//placement new
#include <cstddef>		//ptrdiff_t, size_t
#include <cstdlib>		//exit()
#include <climits>		//UINT_MAX
#include <iostream>		//cerr

namespace JJ
{
	template <class T>
	inline T* _allocate(ptrdiff_t size, T*)
	{
		//设置新的内存分配失败处理程序，使用 0 表示不调用任何程序，直接返回空指针
		std::set_new_handler(0);

		//使用全局 operator new 来分配内存
		T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
		if (tmp == 0)
		{
			std::cerr << "out of memory" << std::endl;
			exit(1);
		}
		return tmp;
	}

	template <class T>
	inline void _deallocate(T* buffer)
	{
		//调用全局 operator delete 来释放内存
		::operator delete(buffer);
	}

	template <class T1, class T2>
	inline void _construct(T1* p, const T2& value)
	{
		new(p) T1(value);
	}

	template <class T>
	inline void _destory(T* ptr)
	{
		ptr->~T();
	}

	template <class T>
	class allocator
	{
	public:
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		// rebind allocator of type U
		template <class U>
		struct rebind
		{
			typedef JJ::allocator<U> other;
		};

		pointer allocate(size_type n, const void* hint = 0)
		{
			return _allocate((difference_type)n, (pointer)0);
		}

		void deallocate(pointer p, size_type n)
		{
			_deallocate(p);
		}

		void construct(pointer p, const T& value)
		{
			_construct(p, value);
		}

		void destory(pointer p)
		{
			_destory(p);
		}

		pointer address(reference x)
		{
			return (pointer)&x;
		}

		const_pointer const_address(const_reference x)
		{
			return (const_reference)&x;
		}

		size_type max_size() const
		{
			return size_type(UINT_MAX / sizeof(T));
		}
	};

}
#endif // !_JJALLOC_