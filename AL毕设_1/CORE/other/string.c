//
//  string.c
//  AL毕设_1
//
//  Created by 郝富杰 on 2020/5/29.
//  Copyright © 2020 郝富杰. All rights reserved.
//

#include "string.h"



//从码点value创建字符串
static Value makeStringFromCodePoint(VM* vm, int value) {
    uint32_t byteNum = getByteNumOfEncodeUtf8(value);
    ASSERT(byteNum != 0, "utf8 encode bytes should be between 1 and 4!");
    
    //+1是为了结尾的'\0'
    ObjString* objString = ALLOCATE_EXTRA(vm, ObjString, byteNum + 1);
    
    if (objString == NULL) {
        MEM_ERROR("allocate memory failed in runtime!");
    }
    
    initObjHeader(vm, &objString->objHeader, OT_STRING, vm->stringClass);
    objString->value.length = byteNum;
    objString->value.start[byteNum] = '\0';
    encodeUtf8((uint8_t*)objString->value.start, value);
    hashObjString(objString);
    return OBJ_TO_VALUE(objString);
}

//用索引index处的字符创建字符串对象
static Value stringCodePointAt(VM* vm, ObjString* objString, uint32_t index) {
    ASSERT(index < objString->value.length, "index out of bound!");
    int codePoint = decodeUtf8((uint8_t*)objString->value.start + index,
                               objString->value.length - index);
    
    //若不是有效的utf8序列,将其处理为单个裸字符
    if (codePoint == -1) {
        return OBJ_TO_VALUE(newObjString(vm, &objString->value.start[index], 1));
    }
    
    return makeStringFromCodePoint(vm, codePoint);
}

//objString.fromCodePoint(_):从码点建立字符串
static bool primStringFromCodePoint(VM* vm, Value* args) {
    
    if (!validateInt(vm, args[1])) {
        return NO;
    }
    
    int codePoint = (int)VALUE_TO_NUM(args[1]);
    if (codePoint < 0) {
        SET_ERROR_FALSE(vm, "code point can`t be negetive!");
    }
    
    if (codePoint > 0x10ffff) {
        SET_ERROR_FALSE(vm, "code point must be between 0 and 0x10ffff!");
    }
    
    RET_VALUE(makeStringFromCodePoint(vm, codePoint));
}

//objString+objString: 字符串相加
static bool primStringPlus(VM* vm, Value* args) {
    if (!validateString(vm, args[1])) {
        return NO;
    }
    
    ObjString* left = VALUE_TO_OBJSTR(args[0]);
    ObjString* right = VALUE_TO_OBJSTR(args[1]);
    
    uint32_t totalLength = strlen(left->value.start) + strlen(right->value.start);
    //+1是为了结尾的'\0'
    ObjString* result = ALLOCATE_EXTRA(vm, ObjString, totalLength + 1);
    if (result == NULL) {
        MEM_ERROR("allocate memory failed in runtime!");
    }
    initObjHeader(vm, &result->objHeader, OT_STRING, vm->stringClass);
    memcpy(result->value.start, left->value.start, strlen(left->value.start));
    memcpy(result->value.start + strlen(left->value.start),
           right->value.start, strlen(right->value.start));
    result->value.start[totalLength] = '\0';
    result->value.length = totalLength;
    hashObjString(result);
    
    RET_OBJ(result);
}

//以utf8编码从source中起始为startIndex,方向为direction的count个字符创建字符串
static ObjString* newObjStringFromSub(VM* vm, ObjString* sourceStr,
                                      int startIndex, uint32_t count, int direction) {
    
    uint8_t* source = (uint8_t*)sourceStr->value.start;
    uint32_t totalLength = 0, idx = 0;
    
    //计算count个utf8编码的字符总共需要的字节数,后面好申请空间
    while (idx < count) {
        totalLength += getByteNumOfDecodeUtf8(source[startIndex + idx * direction]);
        idx++;
    }
    
    //+1是为了结尾的'\0'
    ObjString* result = ALLOCATE_EXTRA(vm, ObjString, totalLength + 1);
    
    if (result == NULL) {
        MEM_ERROR("allocate memory failed in runtime!");
    }
    initObjHeader(vm, &result->objHeader, OT_STRING, vm->stringClass);
    result->value.start[totalLength] = '\0';
    result->value.length = totalLength;
    
    uint8_t* dest = (uint8_t*)result->value.start;
    idx = 0;
    while (idx < count) {
        int index = startIndex + idx * direction;
        //解码,获取字符数据
        int codePoint = decodeUtf8(source + index, sourceStr->value.length - index);
        if (codePoint != -1) {
            //再将数据按照utf8编码,写入result
            dest += encodeUtf8(dest, codePoint);
        }
        idx++;
    }
    
    hashObjString(result);
    return result;
}

//objString[_]:用数字或objRange对象做字符串的subscript
static bool primStringSubscript(VM* vm, Value* args) {
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    //数字和objRange都可以做索引,分别判断
    //若索引是数字,就直接索引1个字符,这是最简单的subscript
    if (VALUE_IS_NUM(args[1])) {
        uint32_t index = validateIndex(vm, args[1], objString->value.length);
        if (index == UINT32_MAX) {
            return NO;
        }
        RET_VALUE(stringCodePointAt(vm, objString, index));
    }
    
    //索引要么为数字要么为ObjRange,若不是数字就应该为objRange
    if (!VALUE_IS_OBJRANGE(args[1])) {
        SET_ERROR_FALSE(vm, "subscript should be integer or range!");
    }
    
    //direction是索引的方向,
    //1表示正方向,从前往后.-1表示反方向,从后往前.
    //from若比to大,即从后往前检索字符,direction则为-1
    int direction;
    
    uint32_t count = objString->value.length;
    //返回的startIndex是objRange.from在objString.value.start中的下标
    uint32_t startIndex = calculateRange(vm, VALUE_TO_OBJRANGE(args[1]), &count, &direction);
    if (startIndex == UINT32_MAX) {
        return NO;
    }
    
    RET_OBJ(newObjStringFromSub(vm, objString, startIndex, count, direction));
}

//objString.byteAt_():返回指定索引的字节
static bool primStringByteAt(VM* vm UNUSED, Value* args) {
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    uint32_t index = validateIndex(vm, args[1], objString->value.length);
    if (index == UINT32_MAX) {
        return NO;
    }
    //故转换为数字返回
    RET_NUM((uint8_t)objString->value.start[index]);
}

//objString.byteCount_:返回字节数
static bool primStringByteCount(VM* vm UNUSED, Value* args) {
    RET_NUM(VALUE_TO_OBJSTR(args[0])->value.length);
}

//objString.codePointAt_(_):返回指定的CodePoint
static bool primStringCodePointAt(VM* vm UNUSED, Value* args) {
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    uint32_t index = validateIndex(vm, args[1], objString->value.length);
    if (index == UINT32_MAX) {
        return NO;
    }
    
    const uint8_t* bytes = (uint8_t*)objString->value.start;
    if ((bytes[index] & 0xc0) == 0x80) {
        //如果index指向的并不是utf8编码的最高字节
        //而是后面的低字节,返回-1提示用户
        RET_NUM(-1);
    }
    
    //返回解码
    RET_NUM(decodeUtf8((uint8_t*)objString->value.start + index,
                       objString->value.length - index));
}

//使用Boyer-Moore-Horspool字符串匹配算法在haystack中查找needle,大海捞针
static int findString(ObjString* haystack, ObjString* needle) {
    //如果待查找的patten为空则为找到
    if (needle->value.length == 0) {
        return 0;    //返回起始下标0
    }
    
    //若待搜索的字符串比原串还长 肯定搜不到
    if (needle->value.length > haystack->value.length) {
        return -1;
    }
    
    //构建"bad-character shift表"以确定窗口滑动的距离
    //数组shift的值便是滑动距离
    uint32_t shift[UINT8_MAX];
    //needle中最后一个字符的下标
    uint32_t needleEnd = needle->value.length - 1;
    
    //一、 先假定"bad character"不属于needle(即pattern),
    //对于这种情况,滑动窗口跨过整个needle
    uint32_t idx = 0;
    while (idx < UINT8_MAX) {
        // 默认为滑过整个needle的长度
        shift[idx] = needle->value.length;
        idx++;
    }
    
    //二、假定haystack中与needle不匹配的字符在needle中之前已匹配过的位置出现过
    //就滑动窗口以使该字符与在needle中匹配该字符的最末位置对齐。
    //这里预先确定需要滑动的距离
    idx = 0;
    while (idx < needleEnd) {
        char c = needle->value.start[idx];
        //idx从前往后遍历needle,当needle中有重复的字符c时,
        //后面的字符c会覆盖前面的同名字符c,这保证了数组shilf中字符是needle中最末位置的字符,
        //从而保证了shilf[c]的值是needle中最末端同名字符与needle末端的偏移量
        shift[(uint8_t)c] = needleEnd - idx;
        idx++;
    }
    
    //Boyer-Moore-Horspool是从后往前比较,这是处理bad-character高效的地方,
    //因此获取needle中最后一个字符,用于同haystack的窗口中最后一个字符比较
    char lastChar = needle->value.start[needleEnd];
    
    //长度差便是滑动窗口的滑动范围
    uint32_t range = haystack->value.length - needle->value.length;
    
    //从haystack中扫描needle,寻找第1个匹配的字符 如果遍历完了就停止
    idx = 0;
    while (idx <= range) {
        //拿needle中最后一个字符同haystack窗口的最后一个字符比较
        //(因为Boyer-Moore-Horspool是从后往前比较), 如果匹配,看整个needle是否匹配
        char c = haystack->value.start[idx + needleEnd];
        if (lastChar == c &&
            memcmp(haystack->value.start + idx, needle->value.start, needleEnd) == 0) {
            //找到了就返回匹配的位置
            return idx;
        }
        
        //否则就向前滑动继续下一伦比较
        idx += shift[(uint8_t)c];
    }
    
    //未找到就返回-1
    return -1;
}

//objString.contains(_):判断字符串args[0]中是否包含子字符串args[1]
static bool primStringContains(VM* vm UNUSED, Value* args) {
    if (!validateString(vm, args[1])) {
        return NO;
    }
    
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    ObjString* pattern = VALUE_TO_OBJSTR(args[1]);
    RET_BOOL(findString(objString, pattern) != -1);
}

//objString.endsWith(_): 返回字符串是否以args[1]为结束
static bool primStringEndsWith(VM* vm UNUSED, Value* args) {
    if (!validateString(vm, args[1])) {
        return NO;
    }
    
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    ObjString* pattern = VALUE_TO_OBJSTR(args[1]);
    
    //若pattern比源串还长,源串必然不包括pattern
    if (pattern->value.length > objString->value.length) {
        RET_FALSE;
    }
    
    char* cmpIdx = objString->value.start +
    objString->value.length - pattern->value.length;
    RET_BOOL(memcmp(cmpIdx, pattern->value.start, pattern->value.length) == 0);
}

//objString.indexOf(_):检索字符串args[0]中子串args[1]的起始下标
static bool primStringIndexOf(VM* vm UNUSED, Value* args) {
    if (!validateString(vm, args[1])) {
        return NO;
    }
    
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    ObjString* pattern = VALUE_TO_OBJSTR(args[1]);
    
    //若pattern比源串还长,源串必然不包括pattern
    if (pattern->value.length > objString->value.length) {
        RET_FALSE;
    }
    
    int index = findString(objString, pattern);
    RET_NUM(index);
}

//objString.iterate(_):返回下一个utf8字符(不是字节)的迭代器
static bool primStringIterate(VM* vm UNUSED, Value* args) {
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    
    //如果是第一次迭代 迭代索引肯定为空
    if (VALUE_IS_NULL(args[1])) {
        if (objString->value.length == 0) {
            RET_FALSE;
        }
        RET_NUM(0);
    }
    
    //迭代器必须是正整数
    if (!validateInt(vm, args[1])){
        return NO;
    }
    
    double iter = VALUE_TO_NUM(args[1]);
    if (iter < 0) {
        RET_FALSE;
    }
    
    uint32_t index = (uint32_t)iter;
    do {
        index++;
        
        //到了结尾就返回NO,表示迭代完毕
        if (index >= objString->value.length) RET_FALSE;
        
        //读取连续的数据字节,直到下一个Utf8的高字节
    } while ((objString->value.start[index] & 0xc0) == 0x80);
    
    RET_NUM(index);
}

//objString.iterateByte_(_): 迭代索引,内部使用
static bool primStringIterateByte(VM* vm UNUSED, Value* args) {
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    
    //如果是第一次迭代 迭代索引肯定为空 直接返回索引0
    if (VALUE_IS_NULL(args[1])) {
        if (objString->value.length == 0) {
            RET_FALSE;
        }
        RET_NUM(0);
    }
    
    //迭代器必须是正整数
    if (!validateInt(vm, args[1])) {
        return NO;
    }
    
    double iter = VALUE_TO_NUM(args[1]);
    
    if (iter < 0) {
        RET_FALSE;
    }
    
    uint32_t index = (uint32_t)iter;
    index++; //移进到下一个字节的索引
    if (index >= objString->value.length) {
        RET_FALSE;
    }
    
    RET_NUM(index);
}



//objString.iteratorValue(_):返回迭代器对应的value
static bool primStringIteratorValue(VM* vm, Value* args) {
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    uint32_t index = validateIndex(vm, args[1], objString->value.length);
    if (index == UINT32_MAX) {
        return NO;
    }
    RET_VALUE(stringCodePointAt(vm, objString, index));
}

//objString.startsWith(_): 返回args[0]是否以args[1]为起始
static bool primStringStartsWith(VM* vm UNUSED, Value* args) {
    if (!validateString(vm, args[1])) {
        return NO;
    }
    
    ObjString* objString = VALUE_TO_OBJSTR(args[0]);
    ObjString* pattern = VALUE_TO_OBJSTR(args[1]);
    
    //若pattern比源串还长,源串必然不包括pattern,
    //因此不可能以pattern为起始
    if (pattern->value.length > objString->value.length) {
        RET_FALSE;
    }
    
    RET_BOOL(memcmp(objString->value.start,
                    pattern->value.start, pattern->value.length) == 0);
}

//objString.toString:获得自己的字符串
static bool primStringToString(VM* vm UNUSED, Value* args) {
    RET_VALUE(args[0]);
}

void bind_string(VM *vm, ObjModule *coreModule)
{
    //字符串类
    vm->stringClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "String"));
    PRIM_METHOD_BIND(vm->stringClass->objHeader.class, "fromCodePoint(_)", primStringFromCodePoint);
    PRIM_METHOD_BIND(vm->stringClass, "+(_)", primStringPlus);
    PRIM_METHOD_BIND(vm->stringClass, "[_]", primStringSubscript);
    PRIM_METHOD_BIND(vm->stringClass, "byteAt_(_)", primStringByteAt);
    PRIM_METHOD_BIND(vm->stringClass, "byteCount_", primStringByteCount);
    PRIM_METHOD_BIND(vm->stringClass, "codePointAt_(_)", primStringCodePointAt);
    PRIM_METHOD_BIND(vm->stringClass, "contains(_)", primStringContains);
    PRIM_METHOD_BIND(vm->stringClass, "endsWith(_)", primStringEndsWith);
    PRIM_METHOD_BIND(vm->stringClass, "indexOf(_)", primStringIndexOf);
    PRIM_METHOD_BIND(vm->stringClass, "iterate(_)", primStringIterate);
    PRIM_METHOD_BIND(vm->stringClass, "iterateByte_(_)", primStringIterateByte);
    PRIM_METHOD_BIND(vm->stringClass, "iteratorValue(_)", primStringIteratorValue);
    PRIM_METHOD_BIND(vm->stringClass, "startsWith(_)", primStringStartsWith);
    PRIM_METHOD_BIND(vm->stringClass, "toString", primStringToString);
    PRIM_METHOD_BIND(vm->stringClass, "count", primStringByteCount);
}

