//
//  utils.c
//  AL
//
//  Created by 郝富杰 on 2019/9/19.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "memory.h"
#include "vm.h"
/*这个库是用来实现函数的可变参数的*/
#include <stdarg.h>


/*
 realloc函数声明：void *realloc(void *ptr, size_t size) 将ptr指向的内存调整为size大小
 realloc 的 三个含义：
 1.如果ptr为NULL并且size不为0，相当于malloc(size),用于申请空间。
 2.如果ptr不为NULL并且size为0，相当于free(ptr),用于释放空间
 3.如果ptr和size都不为0，用于申请空间。空间申请有两种可能：
    1)在原有空间上的基础上继续分配新空间，内存地址和之前一样
    2)重新分配一块新的空间，相当于先free(ptr),然后再malloc(size)
    所以基于这点要注意的是：如果使用realloc，不要把relloc的返回值赋值给原指针，否则relloc申请空间失败，相当于用NULL去替换了原指针。
 */
void *memManager(VM *vm, void *ptr, uint32_t oldSize, uint32_t newSize)
{
    //调整虚拟机已分配的空间
    vm->allocatedBytes += newSize - oldSize;
    
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }
    
    //如果ptr为NULL，newSize为0，会返回一个非空的新指针
    return realloc(ptr, newSize);
}

/*
 1.首先将该问题转换为对数思考 log(2)V = x
    当x为整数时，V的大于等于的最近2次幂为本身.          2^x = V'
    当x不为整数时，V的大于等于的最近2次幂为(int)x + 1.    2^((int)x+1)=V'
 2.统一化思考此问题，提出将原公式改为 (log(2)(V-1)+1) = x
    当V本身是2次幂时，2^(int)x = V'
    当V不是2次幂是，2^(int)x = V'
 3.用计算机表示这个思路
    借助数学函数  效率低
    使用二进制移位操作   效率高（直接操作二进制）
 4.实现这个过程(以8位二进制为例,不考虑最高位为符号位)
    01000001
         - 1
    01000000 (对应(V-1))   将最高位为1的最后面全部变1,然后+1(对应log+1取整)
 5.变1的过程，通过移位操作
    01000000
        左移1
    00100000
           |
    01100000       现在有两个1，下次移位2个与这个|操作，就会有4个1，再下次移位4个（也可以3个，但为了效率且已经保证有4个连续的1)
    ，就会有8个1（是7个，因为只有7位），然后+1
    10000000    128   原数65，最近2次幂即为128
 6.对于int而言，有32位，移位1 2 4 8 16刚好可以实现全1,（函数实现） 那么对于long而言，就是最后再移32位
 */
uint32_t ceilToPowerOf2(uint32_t v)
{
    v += (v==0);
    v--;
    
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

DEFINE_BUFFER_METHOD(String)
DEFINE_BUFFER_METHOD(Int)
DEFINE_BUFFER_METHOD(Char)
DEFINE_BUFFER_METHOD(Byte)



void symbolTableClear(VM *vm, SymbolTable *buffer)
{
    
    uint32_t idx = 0;
    while (idx < buffer->count) {
        memManager(vm, buffer->datas[idx++].str, 0, 0);
    }
    
    StringBufferClear(vm, buffer);
}



















