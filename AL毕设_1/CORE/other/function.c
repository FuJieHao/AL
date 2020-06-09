//
//  function.c
//  AL毕设_1
//
//  Created by 郝富杰 on 2020/5/29.
//  Copyright © 2020 郝富杰. All rights reserved.
//

#include "function.h"

//Fn.new(_):新建一个函数对象
static bool primFnNew(VM* vm, Value* args) {
    //代码块为参数必为闭包
    if (!validateFn(vm, args[1])) return NO;
    
    //直接返回函数闭包
    RET_VALUE(args[1]);
}

void bind_func(VM *vm, ObjModule *coreModule)
{
    //绑定函数类
    vm->fnClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Fn"));
    PRIM_METHOD_BIND(vm->fnClass->objHeader.class, "new(_)", primFnNew);
}


