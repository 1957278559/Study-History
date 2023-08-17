/*
* ���ļ����ڴ������͵�����ŵĺ궨��
* ��Ҫ����֧�� LevelDB �Ķ�̬�⹦�ܣ��Ա��ڲ�ͬƽ̨�ͱ���������ȷ�ĵ����͵���
* �����ͱ���
*/

#ifndef STORAGE_LEVELDB_INCLUDE_EXPORT_H_
#define STORAGE_LEVELDB_INCLUDE_EXPORT_H_

// LEVELED_EXPORT ���ڱ����Ҫ�ڶ�̬���е����ĺ����ͱ���
// windows ƽ̨ʹ�� __declspec(dllexport) �������ͱ������Ϊ����
// linux ƽ̨ʹ�� __attribute__((visibility("default"))) ����ǵ���
// ���ڲ���Ҫ�����ĺ����ͱ�����������ƽ̨���ᱻ���Ϊ��
//#if !defined(LEVELDB_EXPORT)

// LEVELDB_SHARED_LIBRARY ������ָʾ�Ƿ����ڱ��� leveldb Ϊ��̬��
// �ڱ��붯̬��ʱ��LEVELDB_SHARED_LIBRARY ��ᱻ���壬����δ����
//#if defined(LEVELDB_SHARED_LIBRARY)
#if defined(_WIN32)

// LEVELDB_COMPILE_LIBRARY �����������Ǳ��붯̬�⻹��ʹ�ö�̬��
// ����Ǳ��붯̬�⣬�걻����Ϊ __declspec(dllexport)�����ڱ�ǵ���
// �����ʹ�ö�̬�⣬��걻����Ϊ __declspec(dllimport)�����ڱ�ǵ���
// ������ƽ̨�ϣ����붯̬��ʱ��ᱻ����Ϊ __attribute__((visibility("default")))
// ����Ϊ��
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