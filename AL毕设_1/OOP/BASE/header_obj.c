//
//  header_obj.c
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "header_obj.h"
#include "vm.h"

DEFINE_BUFFER_METHOD(Value)

void initObjHeader(VM *vm, ObjHeader *objHeader, ObjType objType, Class *class)
{
    objHeader->type = objType;
    objHeader->isDark = NO;
    objHeader->class = class;
    
    //将初始化的objHeader插入到链表头
    objHeader->next = vm->allObjects;
    vm->allObjects = objHeader;
}
