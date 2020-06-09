//
//  obj_range.h
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef obj_range_h
#define obj_range_h

#include "class.h"

//隐式对象， ..创建 ，没有new形式。 且范围是双闭区间的。
/*
 from起始  to终点。
 to -1 尾元素
 from > to,逆序
 from = to,取值
 */
typedef struct {
    ObjHeader objHeader;
    int from;   //范围的起始
    int to;     //范围的结束
}ObjRange;

ObjRange *newObjRange(VM *vm, int from, int to);

#endif /* obj_range_h */
