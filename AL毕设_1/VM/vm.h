//
//  vm.h
//  AL
//
//  Created by 郝富杰 on 2019/9/19.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef vm_h
#define vm_h

#include "common.h"
#include "class.h"
#include "obj_map.h"
#include "obj_thread.h"
#include "parser.h"

//为定义在opcode.inc中的操作码加上前缀"OPCODE_"
#define OPCODE_SLOTS(opcode, effect) OPCODE_##opcode,
typedef enum {
   #include "../COMPILER/opcode.inc"
} OpCode;
#undef OPCODE_SLOTS

typedef enum vmResult {
   VM_RESULT_SUCCESS,
   VM_RESULT_ERROR
} VMResult;   //虚拟机执行结果
//如果执行无误,可以将字符码输出到文件缓存,避免下次重新编译

struct vm {
    Class *classOfClass;
    Class *objectClass;
    
    Class *stringClass;
    Class *mapClass;
    Class *rangeClass;
    Class *listClass;
    
    //数字也是个对象，但是没有必要用复杂的结构去处理它，但必须为其找一个归属类。这样才能使用相应类中的方法。
    //vm->numClass就是其归属类
    Class *boolClass;
    Class *numClass;
    Class *nullClass;
    
    Class *fnClass;
    
    Class *threadClass;
    
    uint32_t allocatedBytes;    //累计分配的内存量
    ObjHeader *allObjects;  //所有已经分配的对象链表
    SymbolTable allMethodNames; //所有类的方法名
    
    ObjThread *curThread;  //当前正在执行的线程
    ObjMap *allModules;     //所有的模块
    Parser *curParser;     //当前词法分析器
};

/// 虚拟机初始化
void initVM(VM *vm);
/// 创建新的虚拟机
VM *newVM(void);
/// 执行指令
VMResult executeInstruction(VM* vm, register ObjThread* curThread);

void ensureStack(VM* vm, ObjThread* objThread, uint32_t neededSlots);
#endif /* vm_h */
