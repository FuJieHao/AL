//
//  class.c
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "class.h"
#include "common.h"
#include "obj_range.h"
#include "vm.h"
#include <string.h>
#include "oop_frame.h"

DEFINE_BUFFER_METHOD(Method)

//创建一个裸类,fieldNum属性个数
Class *newRawClass(VM *vm, const char *name, uint32_t fieldNum)
{
    Class *class = ALLOCATE(vm, Class);
    
    //裸类没有元类
    initObjHeader(vm, &class->objHeader, OT_CLASS, NULL);
    class->name = newObjString(vm, name, (int)strlen(name));
    class->fieldNum = fieldNum;
    class->superClass = NULL;   //默认没有基类
    MethodBufferInit(&class->methods);
    
    return class;
}

//创建一个类
Class* newClass(VM* vm, ObjString* className, uint32_t fieldNum, Class* superClass) {
    //10表示strlen(" metaClass"
#define MAX_METACLASS_LEN MAX_ID_LEN + 10
    char newClassName[MAX_METACLASS_LEN] = {'\0'};
#undef MAX_METACLASS_LEN
    
    memcpy(newClassName, className->value.start, className->value.length);
    memcpy(newClassName + className->value.length, " metaclass", 10);
    
    //先创建子类的meta类
    Class* metaclass = newRawClass(vm, newClassName, 0);
    metaclass->objHeader.class = vm->classOfClass;
    
    //绑定classOfClass为meta类的基类
    //所有类的meta类的基类都是classOfClass
    bindSuperClass(vm, metaclass, vm->classOfClass);
    
    //最后再创建类
    memcpy(newClassName, className->value.start, className->value.length);
    newClassName[className->value.length] = '\0';
    Class* class = newRawClass(vm, newClassName, fieldNum);
    
    class->objHeader.class = metaclass;
    bindSuperClass(vm, class, superClass);
    
    return class;
}

//数字等Value也被视为对象，因此参数为Value,获得对象obj所属的类
inline Class *getClassOfObj(VM *vm, Value object)
{
    switch (object.type) {
        case VT_NULL:
            return vm->nullClass;
        case VT_FALSE:
        case VT_TRUE:
            return vm->boolClass;
        case VT_NUM:
            return vm->numClass;
        case VT_OBJ:
            return VALUE_TO_OBJ(object)->class;
            
        default:
            NOT_REACHED();
    }
    return NULL;
}

//判断a和b是否相等
bool valueIsEqual(Value v1, Value v2)
{
    //类型不同则无需进行后面的比较
    if (v1.type != v2.type) {
        return NO;
    }
    
    //同为数字，比较数值
    if (v1.type == VT_NUM) {
        return v1.num == v2.num;
    }
    
    //同为对象，且所指的对象是同一个则返回YES
    if (v1.objHeader == v2.objHeader) {
        return YES;
    }
    
    //对象类型不同无需比较
    if (v1.objHeader->type != v2.objHeader->type) {
        return NO;
    }
    
    //以下处理类型相同的对象
    //若对象同为字符串
    if (v1.objHeader->type == OT_STRING) {
        ObjString *strA = VALUE_TO_OBJSTR(v1);
        ObjString *strB = VALUE_TO_OBJSTR(v2);
        //字符串是否相同
        return (strA->value.length == strB->value.length &&
                memcmp(strA->value.start, strB->value.start, strA->value.length) == 0);
    }
    
    //若对象同为range
    if (v1.objHeader->type == OT_RANGE) {
        ObjRange *rgA = VALUE_TO_OBJRANGE(v1);
        ObjRange *rgB = VALUE_TO_OBJRANGE(v2);
        return (rgA->from == rgB->from && rgA->to == rgB->to);
    }
    
    return NO;
}

