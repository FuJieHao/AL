//
//  obj_string.c
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "obj_string.h"
#include <string.h>
#include "vm.h"
#include "memory.h"
#include "common.h"
#include <stdlib.h>

//fnv-1a哈希算法
/*
 hash = FNV_offset_basis
 for each byte_of_data to be hashed
 hash = hash ^ byte_of_data
 hash = hash * FNV_prime
 return hash
 
 FNV_offset_basis hash的首次填充值
 32位 2166136261
 FNV_prime
 32位 16777619
 */
uint32_t hashString(char *str, uint32_t length)
{
    uint32_t hashCode = 2166136261, idx = 0;
    while (idx < length) {
        hashCode ^= str[idx];
        hashCode *= 16777619;
        idx++;
    }
    return hashCode;
}

//为string计算哈希并将值存储到string->hash
void hashObjString(ObjString *objString)
{
    objString->hashCode =
        hashString(objString->value.start, objString->value.length);
}

//以str字符串创建ObjString对象，允许空串""
ObjString *newObjString(VM *vm, const char *str, uint32_t length)
{
    //length 为 0 时 str必为NULL ,length 不为 0时str 不为 NULL
    ASSERT(length == 0 || str != NULL, "str length don't match str!");
    
    //+1是为了结尾的'\0'
    ObjString *objString = ALLOCATE_EXTRA(vm, ObjString, length + 1);
    
    if (objString != NULL) {
        initObjHeader(vm, &objString->objHeader, OT_STRING, vm->stringClass);
        objString->value.length = length;
        
        //支持空字符串: str 为 null, length 为 0
        //如果非空则复制其内容
        if (length > 0) {
            memcpy(objString->value.start, str, length);
        }
        objString->value.start[length] = '\0';
        hashObjString(objString);
    } else {
        MEM_ERROR("Allocating ObjString failed!");
    }

    return objString;
}







