//
//  obj_thread.h
//  AL
//
//  Created by 郝富杰 on 2019/9/29.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef obj_thread_h
#define obj_thread_h

#include "obj_fn.h"

typedef struct objThread{
    
    ObjHeader objHeader;
    
    Value *stack;   //运行时的栈底
    Value *esp;     //运行时的栈顶
    uint32_t stackCapacity; //栈容量
    
    Frame *frames;   //调用框架,函数运行时栈数组
    uint32_t usedFrameNum;  //已使用的frame数量
    uint32_t frameCapacity; //frame容量
    
    // "打开的upvalue"的链表首节点，包括closedUpValue 和 openUpValue
    ObjUpvalue *openUpvalues;
    
    //当前thread的调用者:若本线程退出，控制权将回到调用者
    struct objThread *caller;
    
    //导致运行时错误的对象会放在此处，否则为空
    Value errorObj;
    
}ObjThread; //线程对象

/// 为运行函数准备帧栈
void prepareFrame(ObjThread *objThread, ObjClosure *objClosure, Value *stackStart);
/// 新建线程
ObjThread *newObjThread(VM *vm, ObjClosure *objClosure);
/// 重置thread
void resetThread(ObjThread *objThread, ObjClosure *objClosure);

#endif /* obj_thread_h */
