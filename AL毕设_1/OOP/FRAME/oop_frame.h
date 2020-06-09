//
//  oop_frame.h
//  AL
//
//  Created by 郝富杰 on 2019/10/2.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef oop_frame_h
#define oop_frame_h

#include "common.h"
#include "meta_obj.h"
#include "class.h"


//#include "compiler.h"

//返回值类型是Value类型，且是放在args[0],args是Value数组
//RET_VALUE的参数就是value类型，无需转换直接赋值
//它是后面"RET_其他类型"的基础
#define RET_VALUE(value)\
do {\
args[0] = value;\
return YES;\
} while(0);

//将obj转换为Value后作为返回值
#define RET_OBJ(objPtr) RET_VALUE(OBJ_TO_VALUE(objPtr))

//将bool值转换为value后作为返回值
#define RET_BOOL(boolean) RET_VALUE(BOOL_TO_VALUE(boolean))

#define RET_NUM(num) RET_VALUE(NUM_TO_VALUE(num))
#define RET_NULL RET_VALUE(VT_TO_VALUE(VT_NULL))
#define RET_TRUE RET_VALUE(VT_TO_VALUE(VT_TRUE))
#define RET_FALSE RET_VALUE(VT_TO_VALUE(VT_FALSE))

//设置线程报错
#define SET_ERROR_FALSE(vmPtr, errMsg) \
do {\
vmPtr->curThread->errorObj = \
OBJ_TO_VALUE(newObjString(vmPtr, errMsg, strlen(errMsg)));\
return NO;\
} while (0);


//绑定方法func到classPtr指向的类
#define PRIM_METHOD_BIND(classPtr, methodName, func) {\
    uint32_t length = strlen(methodName);\
    int globalIdx = getIndexFromSymbolTable(&vm->allMethodNames, methodName, length);\
    if (globalIdx == -1) {\
        globalIdx = addSymbol(vm, &vm->allMethodNames, methodName, length);\
    }\
    Method method;\
    method.type = MT_PRIMITIVE;\
    method.primFn = func;\
    bindMethod(vm, classPtr, (uint32_t)globalIdx, method);\
}

//使class->methods[index]=method
void bindMethod(VM *vm, Class *class, uint32_t index, Method method);

//绑定基类
void bindSuperClass(VM *vm, Class *subClass, Class *superClass);

void createOopFrame(VM *vm, ObjModule *coreModule);

#endif /* oop_frame_h */
