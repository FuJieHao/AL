//
//  obj_thread.c
//  AL
//
//  Created by 郝富杰 on 2019/9/29.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "obj_thread.h"
#include "vm.h"
#include "class.h"

//为运行函数准备帧栈，objClosure闭包，包括待运行的函数
void prepareFrame(ObjThread *objThread, ObjClosure *objClosure, Value *stackStart)
{
    ASSERT(objThread->frameCapacity > objThread->usedFrameNum, "frame not enough!");
    //objThread->usedFrameNum是最新可用的frame
    Frame *frame = &(objThread->frames[objThread->usedFrameNum++]);
    
    //thread中各个frame是共享thread的stack
    //frame用frame->stackStart指向各自frame在thread->stack中的起始位置
    frame->stackStart = stackStart;
    frame->closure = objClosure;
    //指令的起始地址是闭包中函数的指令流的起始地址
    frame->ip = objClosure->fn->instrStream.datas;
}

//重置thread
void resetThread(ObjThread *objThread, ObjClosure *objClosure)
{
    objThread->esp = objThread->stack;
    objThread->openUpvalues = NULL;
    objThread->caller = NULL;
    objThread->errorObj = VT_TO_VALUE(VT_NULL);
    objThread->usedFrameNum = 0;
    
    ASSERT(objClosure != NULL, "objClosure is NULL in function resetThread!");
    prepareFrame(objThread, objClosure, objThread->stack);
}

//新建线程
ObjThread *newObjThread(VM *vm, ObjClosure *objClosure)
{
    ASSERT(objClosure != NULL, "objClosure is NULL!");
    
    //为frames数组分配内存
    Frame *frames = ALLOCATE_ARRAY(vm, Frame, INITIAL_FRAME_NUM);
    
    //计算线程容量stackCapacity, +1是为了在栈底存储消息的接受者，即类class或this对象
    //实例方法的接受者是对象，类方法的接受者是类的meta类
    uint32_t stackCapacity = ceilToPowerOf2(objClosure->fn->maxStackSlotUsedNum + 1);
    //分配线程栈的内存空间
    Value *newStack = ALLOCATE_ARRAY(vm, Value, stackCapacity);
    
    //创建线程初始化对象头
    ObjThread *objThread = ALLOCATE(vm, ObjThread);
    initObjHeader(vm, &objThread->objHeader, OT_THREAD, vm->threadClass);
    
    objThread->frames = frames;
    objThread->frameCapacity = INITIAL_FRAME_NUM;
    objThread->stack = newStack;
    objThread->stackCapacity = stackCapacity;
    
    resetThread(objThread, objClosure);
    return objThread;
}

