//
//  obj_list.h
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef obj_list_h
#define obj_list_h

#include "class.h"
#include "vm.h"

typedef struct {
    ObjHeader objHeader;
    ValueBuffer elements;   //存储列表中的所有元素
}ObjList;   //list对象

/// 新建list对象，元素个数为elementNum
ObjList *newObjList(VM *vm, uint32_t elementNum);

Value removeElement(VM *vm, ObjList *objList, uint32_t index);
void insertElement(VM *vm, ObjList *objList, uint32_t index, Value value);

#endif /* obj_list_h */
