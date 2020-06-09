//
//  core_common.c
//  AL毕设_1
//
//  Created by 郝富杰 on 2020/5/29.
//  Copyright © 2020 郝富杰. All rights reserved.
//

#include "core_common.h"
#include <math.h>

//table中查找符号symbol 找到后返回索引,否则返回-1
int getIndexFromSymbolTable(SymbolTable* table, const char* symbol, uint32_t length) {
    ASSERT(length != 0, "length of symbol is 0!");
    uint32_t index = 0;
    while (index < table->count) {
        if (length == table->datas[index].length &&
            memcmp(table->datas[index].str, symbol, length) == 0) {
            return index;
        }
        index++;
    }
    return -1;
}

//返回核心类name的value结构
Value getCoreClassValue(ObjModule* objModule, const char* name) {
    int index = getIndexFromSymbolTable(&objModule->moduleVarName, name, strlen(name));
    if (index == -1) {
        char id[MAX_ID_LEN] = {'\0'};
        memcpy(id, name, strlen(name));
        RUN_ERROR("something wrong occur: missing core class \"%s\"!", id);
    }
    return objModule->moduleVarValue.datas[index];
}

//校验arg是否为函数
bool validateFn(VM* vm, Value arg) {
    if (VALUE_TO_OBJCLOSURE(arg)) {
        return YES;
    }
    vm->curThread->errorObj =
    OBJ_TO_VALUE(newObjString(vm, "argument must be a function!", 28));
    return NO;
}

//往table中添加符号symbol,返回其索引
int addSymbol(VM* vm, SymbolTable* table, const char* symbol, uint32_t length) {
    ASSERT(length != 0, "length of symbol is 0!");
    String string;
    string.str = ALLOCATE_ARRAY(vm, char, length + 1);
    memcpy(string.str, symbol, length);
    string.str[length] = '\0';
    string.length = length;
    StringBufferAdd(vm, table, string);
    return table->count - 1;
}

//切换到下一个线程nextThread
bool switchThread(VM* vm,
                         ObjThread* nextThread, Value* args, bool withArg) {
    //在下一线程nextThread执行之前,其主调线程应该为空
    if (nextThread->caller != NULL) {
        RUN_ERROR("thread has been called!");
    }
    nextThread->caller = vm->curThread;
    
    if (nextThread->usedFrameNum == 0) {
        //只有已经运行完毕的thread的usedFrameNum才为0
        SET_ERROR_FALSE(vm, "a finished thread can`t be switched to!");
    }
    
    if (!VALUE_IS_NULL(nextThread->errorObj)) {
        //Thread.abort(arg)会设置errorObj, 不能切换到abort的线程
        SET_ERROR_FALSE(vm, "a aborted thread can`t be switched to!");
    }
    
    //如果call有参数,回收参数的空间,
    //只保留次栈顶用于存储nextThread返回后的结果
    if (withArg) {
        vm->curThread->esp--;
    }
    
    ASSERT(nextThread->esp > nextThread->stack, "esp should be greater than stack!");
    //nextThread.call(arg)中的arg做为nextThread.yield的返回值
    //存储到nextThread的栈顶,否则压入null保持栈平衡
    nextThread->esp[-1] = withArg ? args[1] : VT_TO_VALUE(VT_NULL);
    
    //使当前线程指向nextThread,使之成为就绪
    vm->curThread = nextThread;
    
    //返回NO以进入vm中的切换线程流程
    return NO;
}

//确保符号已添加到符号表
int ensureSymbolExist(VM* vm, SymbolTable* table, const char* symbol, uint32_t length) {
    int symbolIndex = getIndexFromSymbolTable(table, symbol, length);
    if (symbolIndex == -1) {
        return addSymbol(vm, table, symbol, length);
    }
    return symbolIndex;
}

//判断arg是否为数字
bool validateNum(VM* vm, Value arg) {
    if (VALUE_IS_NUM(arg)) {
        return YES;
    }
    SET_ERROR_FALSE(vm, "argument must be number!");
}

//判断arg是否为字符串
bool validateString(VM* vm, Value arg) {
    if (VALUE_IS_OBJSTR(arg)) {
        return YES;
    }
    SET_ERROR_FALSE(vm, "argument must be string!");
}

//确认value是否为整数
bool validateIntValue(VM* vm, double value) {
    if (trunc(value) == value) {
        return YES;
    }
    SET_ERROR_FALSE(vm, "argument must be integer!");
}

//校验参数index是否是落在"[0, length)"之间的整数
uint32_t validateIndexValue(VM* vm, double index, uint32_t length) {
    //索引必须是数字
    if (!validateIntValue(vm, index)) {
        return UINT32_MAX;
    }
    
    //支持负数索引,负数是从后往前索引
    //转换其对应的正数索引.如果校验失败则返回UINT32_MAX
    if (index < 0) {
        index += length;
    }
    
    //索引应该落在[0,length)
    if (index >= 0 && index < length) {
        return (uint32_t)index;
    }
    
    //执行到此说明超出范围
    vm->curThread->errorObj =
    OBJ_TO_VALUE(newObjString(vm, "index out of bound!", 19));
    return UINT32_MAX;
}

//验证index有效性
uint32_t validateIndex(VM* vm, Value index, uint32_t length) {
    if (!validateNum(vm, index)) {
        return UINT32_MAX;
    }
    return validateIndexValue(vm, VALUE_TO_NUM(index), length);
}

//校验arg是否为整数
bool validateInt(VM* vm, Value arg) {
    //首先得是数字
    if (!validateNum(vm, arg)) {
        return NO;
    }
    
    //再校验数值
    return validateIntValue(vm, VALUE_TO_NUM(arg));
}

//计算objRange中元素的起始索引及索引方向
uint32_t calculateRange(VM* vm,
                               ObjRange* objRange, uint32_t* countPtr, int* directionPtr) {
    
    uint32_t from = validateIndexValue(vm, objRange->from, *countPtr);
    if (from == UINT32_MAX) {
        return UINT32_MAX;
    }
    
    uint32_t to = validateIndexValue(vm, objRange->to, *countPtr);
    if (to == UINT32_MAX) {
        return UINT32_MAX;
    }
    
    //如果from和to为负值,经过validateIndexValue已经变成了相应的正索引
    *directionPtr = from < to ? 1 : -1;
    *countPtr = abs((int)(from - to)) + 1;
    return from;
}


