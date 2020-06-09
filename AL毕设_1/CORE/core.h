//
//  core.h
//  AL
//
//  Created by 郝富杰 on 2019/9/29.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef core_h
#define core_h

#include "vm.h"
#include "file_read.h"

//执行模块，脚本被视为一个模块，脚本的执行调用该函数
VMResult executeModule(VM *vm, Value moduleName, const char*moduleCode);

//编译核心模块
void buildCore(VM *vm);

//从table中查找符号symbol 找到后返回索引，否则返回-1
int getIndexFromSymbolTable(SymbolTable *table, const char *symbol, uint32_t length);

//往table中添加符号symbol, 返回其索引
int addSymbol(VM *vm, SymbolTable *table, const char *symbol, uint32_t length);

//确保符号已添加到符号表
int ensureSymbolExist(VM *vm, SymbolTable *table, const char *symbol, uint32_t length);


#endif /* core_h */
