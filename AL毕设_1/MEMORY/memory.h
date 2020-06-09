//
//  utils.h
//  AL
//
//  Created by 郝富杰 on 2019/9/19.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "common.h"

/*
 该函数为系统的内存管理器，负责内存的分配、扩容和释放
 **/
void *memManager(VM *vm, void *ptr, uint32_t oldSize, uint32_t newSize);

// 该宏(ALLOCATE)的功能是分配类型为type的内存块
#define ALLOCATE(vmPtr, type) \
    (type *)memManager(vmPtr, NULL, 0, sizeof(type))

// 该宏(ALLOCATE_EXTRA)用于分配除主类型mainType外还需要额外的extraSize的内存，用于实现柔性数组的功能。
#define ALLOCATE_EXTRA(vmPtr, mainType, extraSize) \
    (mainType *)memManager(vmPtr, NULL, 0, sizeof(mainType) + extraSize)

// 该宏(ALLOCATE_ARRAY)用于分配数组内存
#define ALLOCATE_ARRAY(vmPtr, type, count) \
    (type *)memManager(vmPtr, NULL, 0, sizeof(type) * count)

// 该宏(DEALLOCATE_ARRAY)用于释放数组内存
#define DEALLOCATE_ARRAY(vmPtr, arrayPtr, count) \
    memManager(vmPtr, arrayPtr, sizeof(arrayPtr[0]) * count, 0)

// 该宏(DEALLOCATE)用于释放内存
#define DEALLOCATE(vmPtr, memPtr) memManager(vmPtr, memPtr, 0, 0)

// 找出大于等于v最近的2次幂
uint32_t ceilToPowerOf2(uint32_t v);

// 定义字符串结构体
typedef struct {
    char *str;
    uint32_t length;
}String;

// 定义CharValue结构体用于存储字符串对象中的字符串(可变)
typedef struct {
    uint32_t length;
    char start[0];
}CharValue;

#define DECLARE_BUFFER_TYPE(type) \
    /*type##Buffer结构体，用于存储类型为type的数据缓冲区*/    \
    typedef struct {\
        /*数据缓冲区*/\
        type *datas;\
        /*缓冲区中已使用的元素个数*/\
        uint32_t count;\
        /*缓冲区容量*/\
        uint32_t capacity;\
    }type##Buffer;\
    /*初始缓冲区buf*/\
    void type##BufferInit(type##Buffer *buf);\
    /*用于往缓冲区中写入fillCount个类型为type的数据data(包含vm的调整)*/\
    void type##BufferFillWrite(VM *vm, type##Buffer *buf, type data, uint32_t fillCount);\
    /*是type##BufferFillWrite的封装，用于往buf中写入1个data*/\
    void type##BufferAdd(VM *vm, type##Buffer *buf, type data);\
    /*用于清空buf,先释放buf->datas内存，然后初始化buf*/\
    void type##BufferClear(VM *vm, type##Buffer *buf);

#define DEFINE_BUFFER_METHOD(type) \
    void type##BufferInit(type##Buffer *buf)\
    {\
        buf->datas = NULL;\
        buf->count = 0;\
        buf->capacity = 0;\
    }\
\
    void type##BufferFillWrite(VM *vm, type##Buffer *buf, type data, uint32_t fillCount)\
    {\
        uint32_t newCounts = buf->count + fillCount;\
        if (newCounts > buf->capacity) {\
            size_t oldSize = buf->capacity * sizeof(type);\
            buf->capacity = ceilToPowerOf2(newCounts);\
            size_t newSize = buf->capacity * sizeof(type);\
            ASSERT(newSize > oldSize, "faint...memory allocate!");\
            buf->datas = (type *)memManager(vm, buf->datas, oldSize, newSize);\
        }\
        uint32_t cnt = 0;\
        while (cnt < fillCount) {\
            buf->datas[buf->count++] = data;\
            cnt++;\
        }\
    }\
\
    void type##BufferAdd(VM *vm, type##Buffer *buf, type data)\
    {\
        type##BufferFillWrite(vm, buf, data, 1);\
    }\
\
    void type##BufferClear(VM *vm, type##Buffer *buf)\
    {\
        size_t oldSize = buf->capacity * sizeof(buf->datas[0]);\
        memManager(vm, buf->datas, oldSize, 0);\
        type##BufferInit(buf);\
    }\

DECLARE_BUFFER_TYPE(String)
// 定义符号表
#define SymbolTable StringBuffer

typedef uint8_t Byte;
typedef char Char;
typedef int Int;
DECLARE_BUFFER_TYPE(Int)
DECLARE_BUFFER_TYPE(Char)
DECLARE_BUFFER_TYPE(Byte)

void StringBufferClear(VM *vm,StringBuffer *buffer);
void symbolTableClear(VM *vm, SymbolTable *buffer);

#define DEFAULT_BUFFER_SIZE 512

#endif /* memory_h */
























