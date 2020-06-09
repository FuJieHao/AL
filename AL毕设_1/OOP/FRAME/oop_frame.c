//
//  oop_frame.c
//  AL
//
//  Created by 郝富杰 on 2019/10/2.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "oop_frame.h"
#include "vm.h"
#include <string.h>
#include "compiler.h"
#include "core.h"


static Class *defineClass(VM *vm, ObjModule *objModule, const char *name);
void bindMethod(VM *vm, Class *class, uint32_t index, Method method);

static bool primObjectNot(VM *vm UNUSED, Value *args);
static bool primObjectEqual(VM *vm UNUSED, Value *args);
static bool primObjectNotEqual(VM *vm UNUSED, Value *args);
static bool primObjectIs(VM *vm, Value *args);
static bool primObjectToString(VM *vm UNUSED, Value *args);
static bool primObjectType(VM *vm, Value *args);

static bool primClassName(VM *vm UNUSED, Value *args);
static bool primClassSupertype(VM *vm UNUSED, Value *args);
static bool primClassToString(VM *vm UNUSED, Value *args);

void bindSuperClass(VM *vm, Class *subClass, Class *superClass);

static bool primObjectmetaSame(VM *vm UNUSED, Value *args);

void createOopFrame(VM *vm, ObjModule *coreModule)
{
    //创建object类并绑定方法
    vm->objectClass = defineClass(vm, coreModule, "object");
    PRIM_METHOD_BIND(vm->objectClass, "!", primObjectNot);
    PRIM_METHOD_BIND(vm->objectClass, "==(_)", primObjectEqual);
    PRIM_METHOD_BIND(vm->objectClass, "!=(_)", primObjectNotEqual);
    PRIM_METHOD_BIND(vm->objectClass, "is(_)", primObjectIs);
    PRIM_METHOD_BIND(vm->objectClass, "toString", primObjectToString);
    PRIM_METHOD_BIND(vm->objectClass, "type", primObjectType);
    
    //定义classOfClass类，它是所有meta类的meta类和基类
    vm->classOfClass = defineClass(vm, coreModule, "class");
    //objectClass是任何类的基类
    bindSuperClass(vm, vm->classOfClass, vm->objectClass);
    
    PRIM_METHOD_BIND(vm->classOfClass, "name", primClassName);
    PRIM_METHOD_BIND(vm->classOfClass, "supertype", primClassSupertype);
    PRIM_METHOD_BIND(vm->classOfClass, "toString", primClassToString);
    
    //定义object类的元信息类objectMetaclass,它无须挂载到vm
    Class *objectMetaclass = defineClass(vm, coreModule, "objectMeta");
    
    //classOfClass类是所有meta类的meta类和基类
    bindSuperClass(vm, objectMetaclass, vm->classOfClass);
    
    //在objectMetaclass绑定类型比较原生方法
    PRIM_METHOD_BIND(objectMetaclass, "same(_,_)", primObjectmetaSame);
    
    //绑定各自的meta类
    vm->objectClass->objHeader.class = objectMetaclass;
    objectMetaclass->objHeader.class = vm->classOfClass;
    vm->classOfClass->objHeader.class = vm->classOfClass;    //元信息类回路，meta类终点
}

//!object:object取反，结果为NO
static bool primObjectNot(VM *vm UNUSED, Value *args)
{
    RET_VALUE(VT_TO_VALUE(VT_FALSE));
}

//args[0] == args[1]:返回object是否相等
static bool primObjectEqual(VM *vm UNUSED, Value *args)
{
    Value boolValue = BOOL_TO_VALUE(valueIsEqual(args[0], args[1]));
    RET_VALUE(boolValue);
}

//args[0] == args[1]:返回object是否不等
static bool primObjectNotEqual(VM *vm UNUSED, Value *args)
{
    Value boolValue = BOOL_TO_VALUE(!valueIsEqual(args[0], args[1]));
    RET_VALUE(boolValue);
}

//args[0] is args[1]:类args[0]是否为类args[1]的子类
static bool primObjectIs(VM *vm, Value *args)
{
    //args[1]必须是class
    if (!VALUE_IS_CLASS(args[1])) {
        RUN_ERROR("argument must be class!");
    }
    
    Class *thisClass = getClassOfObj(vm, args[0]);
    Class *baseClass = (Class *)(args[1].objHeader);
    
    //有可能是多级继承，因此自下而上遍历其基类链
    while (baseClass != NULL) {
        //在某一级基类找到匹配，就设置返回值为VT_TRUE并返回
        if (thisClass == baseClass) {
            RET_VALUE(VT_TO_VALUE(VT_TRUE));
        }
        baseClass = baseClass->superClass;
    }
    
    //若未找到基类，说明不具备is_a关系
    RET_VALUE(VT_TO_VALUE(VT_FALSE));
}

//args[0].toString:返回args[0]所属class的名字
static bool primObjectToString(VM *vm UNUSED, Value *args)
{
    Class *class = args[0].objHeader->class;
    Value nameValue = OBJ_TO_VALUE(class->name);
    RET_VALUE(nameValue);
}

//args[0].type:返回对象args[0]的类
static bool primObjectType(VM *vm, Value *args)
{
    Class *class = getClassOfObj(vm, args[0]);
    RET_OBJ(class);
}

//args[0].name:返回类名
static bool primClassName(VM *vm UNUSED, Value *args)
{
    RET_OBJ(VALUE_TO_CLASS(args[0])->name);
}

//args[0].supertype:返回args[0]的基类
static bool primClassSupertype(VM *vm UNUSED, Value *args)
{
    Class *class = VALUE_TO_CLASS(args[0]);
    if (class->superClass != NULL) {
        RET_OBJ(class->superClass);
    }
    RET_VALUE(VT_TO_VALUE(VT_NULL));
}

//args[0].toString:返回类名
static bool primClassToString(VM *vm UNUSED, Value *args)
{
    RET_OBJ(VALUE_TO_CLASS(args[0])->name);
}

//args[0].same(args[1], args[2]):返回args[1]和args[2]是否相等
static bool primObjectmetaSame(VM *vm UNUSED, Value *args)
{
    Value boolValue = BOOL_TO_VALUE(valueIsEqual(args[1], args[2]));
    RET_VALUE(boolValue);
}


//定义类
static Class *defineClass(VM *vm, ObjModule *objModule, const char *name)
{
    //1.先创建类
    Class *class = newRawClass(vm, name, 0);
    
    //2.把类作为普通变量在模块中定义
    defineModuleVar(vm, objModule, name, strlen(name), OBJ_TO_VALUE(class));
    return class;
}

//使class->methods[index]=method
void bindMethod(VM *vm, Class *class, uint32_t index, Method method)
{
    //使各类自己的methods数组和vm->allmethodsNames保持同样的长度，用空占位填充
    if (index >= class->methods.count) {
        Method emptyPad = {MT_NONE,{0}};
        MethodBufferFillWrite(vm, &class->methods, emptyPad, index - class->methods.count + 1);
    }
    class->methods.datas[index] = method;
}

//绑定基类
void bindSuperClass(VM *vm, Class *subClass, Class *superClass)
{
    subClass->superClass = superClass;
    
    //继承基类属性数
    subClass->fieldNum += superClass->fieldNum;
    
    //继承基类方法
    uint32_t idx = 0;
    while (idx < superClass->methods.count) {
        bindMethod(vm, subClass, idx, superClass->methods.datas[idx]);
        idx++;
    }
}

