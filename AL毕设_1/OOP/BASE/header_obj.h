//
//  header_obj.h
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef header_obj_h
#define header_obj_h

#include "memory.h"

typedef enum {
    OT_CLASS,   //只有这个是class类型，下面的都是object对象
    OT_LIST,    //list类型，列表
    OT_MAP,     //map类型，散列数组
    OT_MODULE,  //module类型，模块作用域
    OT_RANGE,   //range类型，表示步长为1的数字范围
    OT_STRING,  //string类型，即字符串
    OT_UPVALUE, //upvalue类型，自由变量
    OT_FUNCTION,    //函数function类型
    OT_CLOSURE,     //闭包类型
    OT_INSTANCE,    //实例，对象实例
    OT_THREAD       //线程类型
}ObjType;   //对象类型

typedef struct objHeader
{
    ObjType type;   //对象类型
    bool isDark;    //对象是否可达
    Class *class;   //对象所属的类
    struct objHeader*next;  //用于连接所有已分配对象
}ObjHeader;     //对象头，用于记录元信息和垃圾回收

typedef enum {
    VT_UNDEFINED,   //表示未定义
    VT_NULL,        //表示空值
    VT_FALSE,       //NO
    VT_TRUE,        //TRUE
    VT_NUM,         //数字类型
    VT_OBJ          //表示值为一个对象
}ValueType;

typedef struct {
    ValueType type;
    union {
        double num;
        ObjHeader *objHeader;
    };
}Value; //通用的值结构

DECLARE_BUFFER_TYPE(Value);

/// 初始化对象头
void initObjHeader(VM *vm, ObjHeader *objHeader, ObjType objType, Class *class);
#endif /* header_obj_h */
