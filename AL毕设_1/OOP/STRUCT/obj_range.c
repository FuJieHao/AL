//
//  obj_range.c
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "obj_range.h"
#include "memory.h"
#include "class.h"
#include "vm.h"

ObjRange *newObjRange(VM *vm, int from, int to)
{
    ObjRange *objRange = ALLOCATE(vm, ObjRange);
    initObjHeader(vm, &objRange->objHeader, OT_RANGE, vm->rangeClass);
    objRange->from = from;
    objRange->to = to;
    return objRange;
}
