/*
* ���ļ������ڲ�ͬƽ̨���ṩ��ƽ̨�ض�ʵ�ֵĳ���ӿ�
* ��������Ԥ�������ѡ�������ͬ��ƽ̨�ض��ļ�
* ����ЩԤ�������Ҫ�������ֲ�ͬ��ƽ̨��������ƽ̨ѡ����ȷ��ʵ���ļ�
*/

#ifndef STORAGE_LEVELDB_PORT_PORT_H_
#define STORAGE_LEVELDB_PORT_PORT_H_

#include <string.h>

// ���� LEVELDB_PLATFORM_POSIX ��� LEVELDB_PLATFORM_WINDOWS ��ѡ��ִ�� port_stdcxx.h �ļ�
// ����������ֱ��ʾ POSIXƽ̨(�� Linux �� macOS ��)�� Windows ƽ̨
//#if defined(LEVELDB_PLATFORM_POSIX) || defined(LEVELDB_PLATFORM_WINDOWS)
//#include "./port_stdcxx.h"

#if defined(LEVELDB_PLATFORM_WINDOWS)
#include "./port_stdcxx.h"

// ����������Ϊ���� Chromium ��Ŀ��ʹ�� leveldb
//#elif defined(LEVELDB_PLATFORM_CHROMIUM)
//#include "./port_chromium.h"
#endif

#endif  // STORAGE_LEVELDB_PORT_PORT_H_