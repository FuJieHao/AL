//
//  core_common.h
//  AL毕设_1
//
//  Created by 郝富杰 on 2020/5/29.
//  Copyright © 2020 郝富杰. All rights reserved.
//

#ifndef core_common_h
#define core_common_h

#include "common.h"
#include "class.h"
#include "vm.h"
#include "oop_frame.h"
#include <string.h>
#include <errno.h>
#include "obj_range.h"

//table中查找符号symbol 找到后返回索引,否则返回-1
int getIndexFromSymbolTable(SymbolTable* table, const char* symbol, uint32_t length);

//返回核心类name的value结构
Value getCoreClassValue(ObjModule* objModule, const char* name);

//校验arg是否为函数
bool validateFn(VM* vm, Value arg);

//往table中添加符号symbol,返回其索引
int addSymbol(VM* vm, SymbolTable* table, const char* symbol, uint32_t length);

//确保符号已添加到符号表
int ensureSymbolExist(VM* vm, SymbolTable* table, const char* symbol, uint32_t length);

//切换到下一个线程nextThread
bool switchThread(VM* vm,
                         ObjThread* nextThread, Value* args, bool withArg);

//判断arg是否为数字
bool validateNum(VM* vm, Value arg);

//判断arg是否为字符串
bool validateString(VM* vm, Value arg);

//确认value是否为整数
bool validateIntValue(VM* vm, double value);

//校验参数index是否是落在"[0, length)"之间的整数
uint32_t validateIndexValue(VM* vm, double index, uint32_t length);

//验证index有效性
uint32_t validateIndex(VM* vm, Value index, uint32_t length);

//校验arg是否为整数
bool validateInt(VM* vm, Value arg);

//计算objRange中元素的起始索引及索引方向
uint32_t calculateRange(VM* vm,
                        ObjRange* objRange, uint32_t* countPtr, int* directionPtr);

#endif /* core_common_h */
