//
//  meta_obj.h
//  AL
//
//  Created by 郝富杰 on 2019/9/25.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef meta_obj_h
#define meta_obj_h

#include "obj_string.h"

//模块：独立作用域中定义的代码集合。 但这其中并不保存“代码”指令流，放在函数对象中保存
//这个只保存在模块中定义的全局变量。
typedef struct {
    ObjHeader objHeader;
    SymbolTable moduleVarName;  //模块中的模块变量名
    ValueBuffer moduleVarValue; //模块中的模块变量值
    ObjString *name;            //模块名
}ObjModule;     //模块对象

typedef struct {
    ObjHeader objHeader;
    //具体的字段
    Value fields[0];
}ObjInstance;   //对象实例

/// 创建一个新的模块对象
ObjModule *newObjModule(VM *vm, const char *modName);
/// 创建一个新的实例化对象
ObjInstance *newObjInstance(VM *vm, Class *class);

#endif /* meta_obj_h */
