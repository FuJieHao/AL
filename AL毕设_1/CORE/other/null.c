//
//  null.c
//  AL毕设_1
//
//  Created by 郝富杰 on 2020/5/29.
//  Copyright © 2020 郝富杰. All rights reserved.
//

#include "null.h"

//null取非
static bool primNullNot(VM* vm UNUSED, Value* args UNUSED) {
    RET_VALUE(BOOL_TO_VALUE(YES));
}

//null的字符串化
static bool primNullToString(VM* vm, Value* args UNUSED) {
    ObjString* objString = newObjString(vm, "null", 4);
    RET_OBJ(objString);
}

void bind_null(VM *vm, ObjModule *coreModule)
{
    //绑定Null类的方法
    vm->nullClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Null"));
    PRIM_METHOD_BIND(vm->nullClass, "!", primNullNot);
    PRIM_METHOD_BIND(vm->nullClass, "toString", primNullToString);
}


