/*
* 该文件用于在不同平台上提供对平台特定实现的抽象接口
* 代码会根据预定义宏来选择包含不同的平台特定文件
* 而这些预定义宏主要用来区分不同的平台，并根据平台选择正确的实现文件
*/

#ifndef STORAGE_LEVELDB_PORT_PORT_H_
#define STORAGE_LEVELDB_PORT_PORT_H_

#include <string.h>

// 根据 LEVELDB_PLATFORM_POSIX 宏和 LEVELDB_PLATFORM_WINDOWS 宏选择执行 port_stdcxx.h 文件
// 上述两个宏分别表示 POSIX平台(如 Linux 和 macOS 等)和 Windows 平台
//#if defined(LEVELDB_PLATFORM_POSIX) || defined(LEVELDB_PLATFORM_WINDOWS)
//#include "./port_stdcxx.h"

#if defined(LEVELDB_PLATFORM_WINDOWS)
#include "./port_stdcxx.h"

// 这个宏可能是为了在 Chromium 项目中使用 leveldb
//#elif defined(LEVELDB_PLATFORM_CHROMIUM)
//#include "./port_chromium.h"
#endif

#endif  // STORAGE_LEVELDB_PORT_PORT_H_