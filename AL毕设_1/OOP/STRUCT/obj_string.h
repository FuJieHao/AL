//
//  obj_string.h
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef obj_string_h
#define obj_string_h

#include "header_obj.h"

typedef struct {
    ObjHeader objHeader;
    uint32_t hashCode;  //字符串的哈希值
    CharValue value;
} ObjString;

/// 对原字符串进行哈希处理
uint32_t hashString(char *str, uint32_t length);
/// 将哈希值存储到字符串对象中
void hashObjString(ObjString *objString);
/// 创建一个新的字符串对象
ObjString *newObjString(VM *vm, const char *str, uint32_t length);
 
#endif /* obj_string_h */
