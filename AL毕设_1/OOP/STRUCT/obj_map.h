//
//  obj_map.h
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef obj_map_h
#define obj_map_h

#include "header_obj.h"

/// 哈希表的容量利用率
#define MAP_LOAD_PERCENT 0.8

typedef struct {
    Value key;
    Value value;
} Entry;    //key-value对

typedef struct {
    ObjHeader objHeader;
    uint32_t capacity;  //Entry容量，包括已经和未使用的Entry数量
    uint32_t count;     //map中使用的Entry的数量
    Entry *entries;     //Entry数组
} ObjMap;

/// 创建新map对象
ObjMap *newObjMap(VM *vm);

void mapSet(VM *vm, ObjMap *objMap, Value key, Value value);
Value mapGet(ObjMap *objMap, Value key);
void clearMap(VM *vm, ObjMap *objMap);
Value removeKey(VM *vm, ObjMap *objMap, Value key);

#endif /* obj_map_h */
