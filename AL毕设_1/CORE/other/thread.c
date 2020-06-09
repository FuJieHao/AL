//
//  thread.c
//  AL毕设_1
//
//  Created by 郝富杰 on 2020/5/29.
//  Copyright © 2020 郝富杰. All rights reserved.
//

#include "thread.h"

//Thread.new(func):创建一个thread实例
static bool primThreadNew(VM* vm, Value* args) {
    //代码块为参数必为闭包
    if (!validateFn(vm, args[1])) {
        return NO;
    }
    
    ObjThread* objThread = newObjThread(vm, VALUE_TO_OBJCLOSURE(args[1]));
    
    //使stack[0]为接收者,保持栈平衡
    objThread->stack[0] = VT_TO_VALUE(VT_NULL);
    objThread->esp++;
    RET_OBJ(objThread);
}

//Thread.abort(err):以错误信息err为参数退出线程
static bool primThreadAbort(VM* vm, Value* args) {
    //此函数后续未处理,暂时放着
    vm->curThread->errorObj = args[1]; //保存退出参数
    return VALUE_IS_NULL(args[1]);
}

//Thread.current:返回当前的线程
static bool primThreadCurrent(VM* vm, Value* args UNUSED) {
    RET_OBJ(vm->curThread);
}

//Thread.suspend():挂起线程,退出解析器
static bool primThreadSuspend(VM* vm, Value* args UNUSED) {
    //目前suspend操作只会退出虚拟机,
    //使curThread为NULL,虚拟机将退出
    vm->curThread = NULL;
    return NO;
}

//Thread.yield(arg)带参数让出cpu
static bool primThreadYieldWithArg(VM* vm, Value* args) {
    ObjThread* curThread = vm->curThread;
    vm->curThread = curThread->caller;   //使cpu控制权回到主调方
    
    curThread->caller = NULL;  //与调用者断开联系
    
    if (vm->curThread != NULL) {
        //如果当前线程有主调方,就将当前线程的返回值放在主调方的栈顶
        vm->curThread->esp[-1] = args[1];
        
        //对于"thread.yield(arg)"来说, 回收arg的空间,
        //保留thread参数所在的空间,将来唤醒时用于存储yield结果
        curThread->esp--;
    }
    return NO;
}

//Thread.yield() 无参数让出cpu
static bool primThreadYieldWithoutArg(VM* vm, Value* args UNUSED) {
    ObjThread* curThread = vm->curThread;
    vm->curThread = curThread->caller;   //使cpu控制权回到主调方
    
    curThread->caller = NULL;  //与调用者断开联系
    
    if (vm->curThread != NULL) {
        //为保持通用的栈结构,如果当前线程有主调方,
        //就将空值做为返回值放在主调方的栈顶
        vm->curThread->esp[-1] = VT_TO_VALUE(VT_NULL) ;
    }
    return NO;
}

//objThread.call()
static bool primThreadCallWithoutArg(VM* vm, Value* args) {
    return switchThread(vm, VALUE_TO_OBJTHREAD(args[0]), args, NO);
}

//objThread.call(arg)
static bool primThreadCallWithArg(VM* vm, Value* args) {
    return switchThread(vm, VALUE_TO_OBJTHREAD(args[0]), args, YES);
}

//objThread.isDone返回线程是否运行完成
static bool primThreadIsDone(VM* vm UNUSED, Value* args) {
    //获取.isDone的调用者
    ObjThread* objThread = VALUE_TO_OBJTHREAD(args[0]);
    RET_BOOL(objThread->usedFrameNum == 0 || !VALUE_IS_NULL(objThread->errorObj));
}

void bind_thread(VM *vm, ObjModule *coreModule)
{
    //Thread类也是在core.script.inc中定义的,
    //将其挂载到vm->threadClass并补充原生方法
    vm->threadClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Thread"));
    //以下是类方法
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "new(_)", primThreadNew);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "abort(_)", primThreadAbort);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "current", primThreadCurrent);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "suspend()", primThreadSuspend);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "yield(_)", primThreadYieldWithArg);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "yield()", primThreadYieldWithoutArg);
    //以下是实例方法
    PRIM_METHOD_BIND(vm->threadClass, "call()", primThreadCallWithoutArg);
    PRIM_METHOD_BIND(vm->threadClass, "call(_)", primThreadCallWithArg);
    PRIM_METHOD_BIND(vm->threadClass, "isDone", primThreadIsDone);
}


