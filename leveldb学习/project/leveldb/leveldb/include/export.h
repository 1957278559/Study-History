/*
* 该文件用于处理导出和导入符号的宏定义
* 主要用来支持 LevelDB 的动态库功能，以便在不同平台和编译器下正确的导出和导入
* 函数和变量
*/

#ifndef STORAGE_LEVELDB_INCLUDE_EXPORT_H_
#define STORAGE_LEVELDB_INCLUDE_EXPORT_H_

// LEVELED_EXPORT 用于标记需要在动态库中导出的函数和变量
// windows 平台使用 __declspec(dllexport) 将函数和变量标记为导出
// linux 平台使用 __attribute__((visibility("default"))) 来标记导出
// 对于不需要导出的函数和变量，在所有平台都会被标记为空
//#if !defined(LEVELDB_EXPORT)

// LEVELDB_SHARED_LIBRARY 宏用于指示是否正在编译 leveldb 为动态库
// 在编译动态库时，LEVELDB_SHARED_LIBRARY 宏会被定义，否则未定义
//#if defined(LEVELDB_SHARED_LIBRARY)
#if defined(_WIN32)

// LEVELDB_COMPILE_LIBRARY 宏用于区分是编译动态库还是使用动态库
// 如果是编译动态库，宏被定义为 __declspec(dllexport)，用于标记导出
// 如果是使用动态库，则宏被定义为 __declspec(dllimport)，用于标记导入
// 在其他平台上，编译动态库时宏会被定义为 __attribute__((visibility("default")))
// 否则为空
#if defined(LEVELDB_COMPILE_LIBRARY)
#define LEVELDB_EXPORT __declspec(dllexport)
#else
#define LEVELDB_EXPORT __declspec(dllimport)
#endif // defined(LEVELDB_COMPILE_LIBRARY)

#else // defined(_WIN32)
//#if defined(LEVELDB_COMPILE_LIBRARY)
//#define LEVELDB_EXPORT __attribute__((visibility("default")))
//#else
#define LEVELDB_EXPORT
//#endif 
#endif	// defined(_WIN32)

//#else	// defined(LEVELDB_SHARED_LIBRARY)
//#define LEVELDB_EXPORT
//#endif

//#endif // !define(LEVELED_EXPORT)

#endif // !STORAGE_LEVELDB_INCLUDE_EXPORT_H_