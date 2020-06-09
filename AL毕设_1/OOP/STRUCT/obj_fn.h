//
//  obj_fn.h
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef obj_fn_h
#define obj_fn_h

#include "memory.h"
#include "meta_obj.h"

typedef struct {
    char *fnName;   //函数名
    IntBuffer lineNo;   //行号
}FnDebug;   //在函数中的调试结构

/*
 这里不单指一般意义的函数，泛指代码单元被翻译成的指令流单元。包括一般的函数，闭包，代码块，模块。
 */
typedef struct {
    ObjHeader objHeader;
    ByteBuffer instrStream;     //函数编译后的指令流
    ValueBuffer constants;      //函数中的常量表
    
    ObjModule *module;      //函数所属的模块
    
    //本函数最多需要的栈空间，是栈使用空间的峰值
    uint32_t maxStackSlotUsedNum;
    uint32_t upvalueNum;    //本函数所涵盖的upvalue数量
    uint8_t argNum; //函数期望的参数个数

#if DEBUG
    FnDebug *debug;
#endif
}ObjFn; //函数对象

/*
 upvalue是内部函数所引用的位于外层函数（包括直接和间接）中的局部变量。
    open upvalue:因为局部变量存储在运行时栈中，如果内层函数所引用的局部变量仍然存储在该局部变量所属的外层函数的运行时栈中，
        那么这个局部变量可以通过运行时栈访问，对于内部函数来说，就称为open upvalue.
    closed upvalue:如果外层函数执行完毕，外层函数的运行时栈被回收，位于栈中的局部变量也会被回收，但内部函数还要使用，
        这个就是闭包的作用，为了处理这个问题，闭包机制会将这个变量从栈中复制到一个安全、且只有内部函数才能访问的栈中，
        这个回收掉的局部变量就称为closed upvalue
 */
typedef struct upvalue{
    ObjHeader objHeader;
    
    //栈是个Value类型的数组，localVarPtr指向upvalue关联的局部变量
    Value *localVarPtr;
    
    //已被关闭的upvalue
    Value closedUpvalue;
    
    struct upvalue *next;   //用以链接openUpvalue链表
}ObjUpvalue;

typedef struct {
    ObjHeader objHeader;
    ObjFn *fn;  //闭包中所引用的函数
    
    ObjUpvalue *upvalues[0];    //用于存储此函数的closed upvalue
}ObjClosure;    //闭包对象

typedef struct {
    uint8_t *ip;    //程序计数器 指向下一个将被执行的指令
    //在本frame中执行的闭包函数
    ObjClosure *closure;
    
    //frame是共享thread.stack
    //此项用于指向本frame所在thread运行时栈的起始地址
    Value *stackStart;
}Frame; //调用框架

/*
 为了在一个线程中运行多个函数，线程中有一个大的运行时栈，该运行时栈被线程中的所有函数共享。
 每个函数的运行时栈都起始于大运行时栈的某个地址，且有一块连续地址。
 */
#define INITIAL_FRAME_NUM 4

/// 创建新的upvalue对象
ObjUpvalue *newObjUpvalue(VM *vm, Value *localVarPtr);
/// 创建新的闭包对象
ObjClosure *newObjClosure(VM *vm, ObjFn *objFn);
/// 创建新的函数对象
ObjFn *newObjFn(VM *vm, ObjModule *objModule, uint32_t maxStackSlotUsedNum);

#endif /* obj_fn_h */





