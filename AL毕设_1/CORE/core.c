//
//  core.c
//  AL
//
//  Created by 郝富杰 on 2019/9/29.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "core.h"
#include "core_common.h"
#include <sys/stat.h>
#include "vm.h"
#include "obj_thread.h"
#include "compiler.h"
#include "core.script.inc"
#include "obj_range.h"
#include "obj_list.h"
#include "obj_map.h"
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "unicodeUtf8.h"

#include "thread.h"
#include "function.h"
#include "overload_call.h"
#include "null.h"
#include "number.h"
#include "string.h"


#define CORE_MODULE VT_TO_VALUE(VT_NULL)

//校验key合法性
static bool validateKey(VM* vm, Value arg) {
   if (VALUE_IS_TRUE(arg)     ||
	 VALUE_IS_FALSE(arg)  ||
	 VALUE_IS_NULL(arg)   ||
	 VALUE_IS_NUM(arg)    || 
	 VALUE_IS_OBJSTR(arg) ||
	 VALUE_IS_OBJRANGE(arg)  ||
	 VALUE_IS_CLASS(arg)) {
	 return YES;
   }
   SET_ERROR_FALSE(vm, "key must be value type!");
}


//从modules中获取名为moduleName的模块
static ObjModule* getModule(VM* vm, Value moduleName) {
   Value value = mapGet(vm->allModules, moduleName);
   if (value.type == VT_UNDEFINED) {
      return NULL;
   }
   return (ObjModule*)(value.objHeader);
}

//载入模块moduleName并编译
static ObjThread* loadModule(VM* vm, Value moduleName, const char* moduleCode) {
   //确保模块已经载入到 vm->allModules
   //先查看是否已经导入了该模块,避免重新导入
   ObjModule* module = getModule(vm, moduleName);

   //若该模块未加载先将其载入,并继承核心模块中的变量
   if (module == NULL) {
      //创建模块并添加到vm->allModules
      ObjString* modName = VALUE_TO_OBJSTR(moduleName);
      ASSERT(modName->value.start[modName->value.length] == '\0', "string.value.start is not terminated!");

      module = newObjModule(vm, modName->value.start);
      mapSet(vm, vm->allModules, moduleName, OBJ_TO_VALUE(module));
      
      //继承核心模块中的变量
      ObjModule* coreModule = getModule(vm, CORE_MODULE);
      uint32_t idx = 0;
      while (idx < coreModule->moduleVarName.count) {
	 defineModuleVar(vm, module,
	       coreModule->moduleVarName.datas[idx].str,
	       strlen(coreModule->moduleVarName.datas[idx].str),
	       coreModule->moduleVarValue.datas[idx]);
	 idx++; 
      }
   }

   ObjFn* fn = compileModule(vm, module, moduleCode);
   ObjClosure* objClosure = newObjClosure(vm, fn);
    
   ObjThread* moduleThread = newObjThread(vm, objClosure);

   return moduleThread;  
}

//获取文件全路径
static char* getFilePath(const char* moduleName) {
   uint32_t rootDirLength = rootDir == NULL ? 0 : (uint32_t)strlen(rootDir);
   uint32_t nameLength = (uint32_t)strlen(moduleName);
   uint32_t pathLength = rootDirLength + nameLength + strlen(".al");
   char* path = (char*)malloc(pathLength + 1);

   if (rootDir != NULL) {
       memmove(path, rootDir, rootDirLength);
   }

   memmove(path + rootDirLength, moduleName, nameLength);
   memmove(path + rootDirLength + nameLength, ".al", 3);
   path[pathLength] = '\0';
   
   return path;
}

//读取模块
static char* readModule(const char* moduleName) {
   //1 读取内建模块  先放着
 
   //2 读取自定义模块
   char* modulePath = getFilePath(moduleName);
   char* moduleCode = readFile(modulePath);
   free(modulePath);

   return moduleCode;  //由主调函数将来free此空间
}

//输出字符串
static void printString(const char* str) {
   //输出到缓冲区后立即刷新
   printf("%s", str);
   fflush(stdout);
}

//导入模块moduleName,主要是把编译模块并加载到vm->allModules
static Value importModule(VM* vm, Value moduleName) {
   //若已经导入则返回NULL_VAL
   if (!VALUE_IS_UNDEFINED(mapGet(vm->allModules, moduleName))) {
      return VT_TO_VALUE(VT_NULL);   
   }
   ObjString* objString = VALUE_TO_OBJSTR(moduleName);
   const char* sourceCode = readModule(objString->value.start);

   ObjThread* moduleThread = loadModule(vm, moduleName, sourceCode);
   return OBJ_TO_VALUE(moduleThread);
}

//在模块moduleName中获取模块变量variableName
static Value getModuleVariable(VM* vm, Value moduleName, Value variableName) {
   //调用本函数前模块已经被加载了
   ObjModule* objModule = getModule(vm, moduleName); 
   if (objModule == NULL) {
      ObjString* modName = VALUE_TO_OBJSTR(moduleName);

      //24是下面sprintf中fmt中除%s的字符个数
      ASSERT(modName->value.length < 512 - 24, "id`s buffer not big enough!");
      char id[512] = {'\0'};
      int len = sprintf(id, "module \'%s\' is not loaded!", modName->value.start);
      vm->curThread->errorObj = OBJ_TO_VALUE(newObjString(vm, id, len));
      return VT_TO_VALUE(VT_NULL);
   }

   ObjString* varName = VALUE_TO_OBJSTR(variableName);

   //从moduleVarName中获得待导入的模块变量
   int index = getIndexFromSymbolTable(&objModule->moduleVarName,
	     varName->value.start, varName->value.length);

   if (index == -1) {
      //32是下面sprintf中fmt中除%s的字符个数
      ASSERT(varName->value.length < 512 - 32, "id`s buffer not big enough!");
      ObjString* modName = VALUE_TO_OBJSTR(moduleName);
      char id[512] = {'\0'};
      int len = sprintf(id, "variable \'%s\' is not in module \'%s\'!",
	    varName->value.start, modName->value.start);
      vm->curThread->errorObj = OBJ_TO_VALUE(newObjString(vm, id, len));
      return VT_TO_VALUE(VT_NULL);
   }

   //直接返回对应的模块变量
   return objModule->moduleVarValue.datas[index];
}


//返回bool的字符串形式:"YES"或"NO"
static bool primBoolToString(VM* vm, Value* args) {
   ObjString* objString;
   if (VALUE_TO_BOOL(args[0])) {  //若为VT_YES
      objString = newObjString(vm, "YES", 4);
   } else {
      objString = newObjString(vm, "NO", 5);
   }
   RET_OBJ(objString);
}

//bool值取反
static bool primBoolNot(VM* vm UNUSED, Value* args) {
   RET_BOOL(!VALUE_TO_BOOL(args[0]));
}

//objList.new():创建1个新的liist
static bool primListNew(VM* vm, Value* args UNUSED) {
   RET_OBJ(newObjList(vm, 0));
}

//objList[_]:索引list元素
static bool primListSubscript(VM* vm, Value* args) {
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);

   //数字和objRange都可以做索引,分别判断
   //若索引是数字,就直接索引1个字符,这是最简单的subscript
   if (VALUE_IS_NUM(args[1])) {
      uint32_t index = validateIndex(vm, args[1], objList->elements.count);
      if (index == UINT32_MAX) {
	 return NO; 
      }
      RET_VALUE(objList->elements.datas[index]);
   }

   //索引要么为数字要么为ObjRange,若不是数字就应该为objRange
   if (!VALUE_IS_OBJRANGE(args[1])) {
      SET_ERROR_FALSE(vm, "subscript should be integer or range!");  
   }

   int direction;

   uint32_t count = objList->elements.count;

   //返回的startIndex是objRange.from在objList.elements.data中的下标
   uint32_t startIndex = calculateRange(vm, VALUE_TO_OBJRANGE(args[1]), &count, &direction);

   //新建一个list 存储该range在原来list中索引的元素
   ObjList* result = newObjList(vm, count);  
   uint32_t idx = 0;
   while (idx < count) {
      //direction为-1表示从后往前倒序赋值
      //如var l = [a,b,c,d,e,f,g]; l[5..3]表示[f,e,d]
      result->elements.datas[idx] = objList->elements.datas[startIndex + idx * direction];   
      idx++;
   }
   RET_OBJ(result);
}

//objList[_]=(_):只支持数字做为subscript
static bool primListSubscriptSetter(VM* vm UNUSED, Value* args) {  
   //获取对象
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);

   //获取subscript
   uint32_t index = validateIndex(vm, args[1], objList->elements.count);
   if (index == UINT32_MAX) {
      return NO; 
   }

   //直接赋值
   objList->elements.datas[index] = args[2];

   RET_VALUE(args[2]); //把参数2做为返回值
}

//objList.add(_):直接追加到list中
static bool primListAdd(VM* vm, Value* args) {
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);
   ValueBufferAdd(vm, &objList->elements, args[1]);
   RET_VALUE(args[1]); //把参数1做为返回值
}

//objList.addCore_(_):编译内部使用的,用于编译列表直接量
static bool primListAddCore(VM* vm, Value* args) {
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);
   ValueBufferAdd(vm, &objList->elements, args[1]);  
   RET_VALUE(args[0]); //返回列表自身
}

//objList.clear():清空list
static bool primListClear(VM* vm, Value* args) {
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);
   ValueBufferClear(vm, &objList->elements);
   RET_NULL;
}

//objList.count:返回list中元素个数
static bool primListCount(VM* vm UNUSED, Value* args) {
   RET_NUM(VALUE_TO_OBJLIST(args[0])->elements.count);
}

//objList.insert(_,_):插入元素
static bool primListInsert(VM* vm, Value* args) {
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);
   //+1确保可以在最后插入
   uint32_t index = validateIndex(vm, args[1], objList->elements.count + 1);
   if (index == UINT32_MAX) {
      return NO; 
   }
   insertElement(vm, objList, index, args[2]);
   RET_VALUE(args[2]);  //参数2做为返回值
}

//objList.iterate(_):迭代list
static bool primListIterate(VM* vm, Value* args) {
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);

   //如果是第一次迭代 迭代索引肯定为空 直接返回索引0
   if (VALUE_IS_NULL(args[1])) {
      if (objList->elements.count == 0) {
	 RET_FALSE;
      }
      RET_NUM(0);
   }

   //确保迭代器是整数
   if (!validateInt(vm, args[1])) {
      return NO; 
   }

   double iter = VALUE_TO_NUM(args[1]);
   //如果迭代完了就终止
   if (iter < 0 || iter >= objList->elements.count - 1) {
      RET_FALSE;
   }
   
   RET_NUM(iter + 1);   //返回下一个
}

//objList.iteratorValue(_):返回迭代值
static bool primListIteratorValue(VM* vm, Value* args) {
   //获取实例对象
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);
   
   uint32_t index = validateIndex(vm, args[1], objList->elements.count);
   if (index == UINT32_MAX) {
      return NO; 
   }

   RET_VALUE(objList->elements.datas[index]);
}

//objList.removeAt(_):删除指定位置的元素
static bool primListRemoveAt(VM* vm, Value* args) {
   //获取实例对象
   ObjList* objList = VALUE_TO_OBJLIST(args[0]);

   uint32_t index = validateIndex(vm, args[1], objList->elements.count);
   if (index == UINT32_MAX) {
      return NO; 
   }

   RET_VALUE(removeElement(vm, objList, index));
}

//objMap.new():创建map对象
static bool primMapNew(VM* vm, Value* args UNUSED) {
   RET_OBJ(newObjMap(vm));
}

//objMap[_]:返回map[key]对应的value
static bool primMapSubscript(VM* vm, Value* args) {
   //校验key的合法性
   if (!validateKey(vm, args[1])) {
      return NO;  //出错了,切换线程
   }

   //获得map对象实例
   ObjMap* objMap = VALUE_TO_OBJMAP(args[0]); 

   //从map中查找key(args[1])对应的value
   Value value = mapGet(objMap, args[1]);

   //若没有相应的key则返回NULL
   if (VALUE_IS_UNDEFINED(value)) {
      RET_NULL;
   }

   RET_VALUE(value);
}

//objMap[_]=(_):map[key]=value
static bool primMapSubscriptSetter(VM* vm, Value* args) {
   //校验key的合法性
   if (!validateKey(vm, args[1])) {
      return NO;  //出错了,切换线程
   }

   //获得map对象实例
   ObjMap* objMap = VALUE_TO_OBJMAP(args[0]); 

   //在map中将key和value关联
   //即map[key]=value
   mapSet(vm, objMap, args[1], args[2]);

   RET_VALUE(args[2]); //返回value 
}

//objMap.addCore_(_,_):编译器编译map字面量时内部使用的,
//在map中添加(key-value)对儿并返回map自身 
static bool primMapAddCore(VM* vm, Value* args) {
   if (!validateKey(vm, args[1])) {
      return NO;  //出错了,切换线程
   }

   //获得map对象实例
   ObjMap* objMap = VALUE_TO_OBJMAP(args[0]); 

   //在map中将key和value关联
   //即map[key]=value
   mapSet(vm, objMap, args[1], args[2]);

   RET_VALUE(args[0]);  //返回map对象自身
}

//objMap.clear():清除map
static bool primMapClear(VM* vm, Value* args) {
   clearMap(vm, VALUE_TO_OBJMAP(args[0]));
   RET_NULL;
}

//objMap.containsKey(_):判断map即args[0]是否包含key即args[1]
static bool primMapContainsKey(VM* vm, Value* args) {
   if (!validateKey(vm, args[1])) {
      return NO;  //出错了,切换线程
   }

   //直接去get该key,判断是否get成功
   RET_BOOL(!VALUE_IS_UNDEFINED(mapGet(VALUE_TO_OBJMAP(args[0]), args[1])));
}

//objMap.count:返回map中entry个数
static bool primMapCount(VM* vm UNUSED, Value* args) {
   RET_NUM(VALUE_TO_OBJMAP(args[0])->count);
}

//objMap.remove(_):删除map[key] map是args[0] key是args[1]
static bool primMapRemove(VM* vm, Value* args) {
   if (!validateKey(vm, args[1])) {
      return NO;  //出错了,切换线程
   }

   RET_VALUE(removeKey(vm, VALUE_TO_OBJMAP(args[0]), args[1]));
}

//objMap.iterate_(_):迭代map中的entry,
//返回entry的索引供keyIteratorValue_和valueIteratorValue_做迭代器
static bool primMapIterate(VM* vm, Value* args) {
   //获得map对象实例
   ObjMap* objMap = VALUE_TO_OBJMAP(args[0]); 

   //map中若空则返回NO不可迭代
   if (objMap->count == 0) {
      RET_FALSE; 
   }

   //若没有传入迭代器,迭代默认是从第0个entry开始
   uint32_t index = 0;  

   //若不是第一次迭代,传进了迭代器
   if (!VALUE_IS_NULL(args[1])) {
      //iter必须为整数
      if (!validateInt(vm, args[1])) {
	 //本线程出错了,返回NO是为了切换到下一线
	 return NO;
      }

      //迭代器不能小0
      if (VALUE_TO_NUM(args[1]) < 0) {
	 RET_FALSE; 
      }

      index = (uint32_t)VALUE_TO_NUM(args[1]);
      //迭代器不能越界
      if (index >= objMap->capacity) {
	 RET_FALSE;  
      }

      index++;  //更新迭代器
   }

   //返回下一个正在使用(有效)的entry
   while (index < objMap->capacity) {
      //entries是个数组, 元素是哈希槽,
      //哈希值散布在这些槽中并不连续,因此逐个判断槽位是否在用
      if (!VALUE_IS_UNDEFINED(objMap->entries[index].key)) {
	 RET_NUM(index);    //返回entry索引
      }
      index++;
   }
   
   //若没有有效的entry了就返回NO,迭代结束
   RET_FALSE;
}

//objMap.keyIteratorValue_(_): key=map.keyIteratorValue(iter)
static bool primMapKeyIteratorValue(VM* vm, Value* args) {
   ObjMap* objMap = VALUE_TO_OBJMAP(args[0]);
   
   uint32_t index = validateIndex(vm, args[1], objMap->capacity);
   if (index == UINT32_MAX) {
      return NO; 
   }

   Entry* entry = &objMap->entries[index];
   if (VALUE_IS_UNDEFINED(entry->key)) {
      SET_ERROR_FALSE(vm, "invalid iterator!");
   }

   //返回该key
   RET_VALUE(entry->key);   
}

//objMap.valueIteratorValue_(_): 
//value = map.valueIteratorValue_(iter)
static bool primMapValueIteratorValue(VM* vm, Value* args) {
   ObjMap* objMap = VALUE_TO_OBJMAP(args[0]);
   
   uint32_t index = validateIndex(vm, args[1], objMap->capacity);
   if (index == UINT32_MAX) {
      return NO; 
   }

   Entry* entry = &objMap->entries[index];
   if (VALUE_IS_UNDEFINED(entry->key)) {
      SET_ERROR_FALSE(vm, "invalid iterator!");
   }

   //返回该key
   RET_VALUE(entry->value);   
}

//objRange.from: 返回range的from
static bool primRangeFrom(VM* vm UNUSED, Value* args) {
   RET_NUM(VALUE_TO_OBJRANGE(args[0])->from);
}

//objRange.to: 返回range的to
static bool primRangeTo(VM* vm UNUSED, Value* args) {
   RET_NUM(VALUE_TO_OBJRANGE(args[0])->to);
}

//objRange.min: 返回range中from和to较小的值
static bool primRangeMin(VM* vm UNUSED, Value* args) {
   ObjRange* objRange = VALUE_TO_OBJRANGE(args[0]);
   RET_NUM(fmin(objRange->from, objRange->to));
}

//objRange.max: 返回range中from和to较大的值
static bool primRangeMax(VM* vm UNUSED, Value* args) {
   ObjRange* objRange = VALUE_TO_OBJRANGE(args[0]);
   RET_NUM(fmax(objRange->from, objRange->to));
}

//objRange.iterate(_): 迭代range中的值,并不索引
static bool primRangeIterate(VM* vm, Value* args) {
   ObjRange* objRange = VALUE_TO_OBJRANGE(args[0]);

   //若未提供iter说明是第一次迭代,因此返回range->from   
   if (VALUE_IS_NULL(args[1])) {
      RET_NUM(objRange->from);
   }

   //迭代器必须是数字
   if (!validateNum(vm, args[1])) {
      return NO;
   }

   //获得迭代器
   double iter = VALUE_TO_NUM(args[1]);

   //若是正方向
   if (objRange->from < objRange->to) {
      iter++;
      if (iter > objRange->to) {
	 RET_FALSE;
      }
   } else {  //若是反向迭代
      iter--; 
      if (iter < objRange->to) {
	 RET_FALSE;
      }
   }

   RET_NUM(iter);
}

//objRange.iteratorValue(_): range的迭代就是range中从from到to之间的值
//因此直接返回迭代器就是range的值
static bool primRangeIteratorValue(VM* vm UNUSED, Value* args) {
   ObjRange* objRange = VALUE_TO_OBJRANGE(args[0]);
   double value = VALUE_TO_NUM(args[1]);

   //确保args[1]在from和to的范围中
   //若是正方向
   if (objRange->from < objRange->to) {
      if (value >= objRange->from && value <= objRange->to) {
	 RET_VALUE(args[1]); 
      }
   } else {  //若是反向迭代
      if (value <= objRange->from && value >= objRange->to) {
	 RET_VALUE(args[1]); 
      }
   }
   RET_FALSE;
}

//System.clock: 返回以秒为单位的系统时钟
static bool primSystemClock(VM* vm UNUSED, Value* args UNUSED) {
   RET_NUM((double)time(NULL));
}

//System.importModule(_): 导入并编译模块args[1],把模块挂载到vm->allModules
static bool primSystemImportModule(VM* vm, Value* args) {
   if (!validateString(vm, args[1])) { //模块名为字符串
      return NO;
   }

   //导入模块name并编译 把模块挂载到vm->allModules
   Value result = importModule(vm, args[1]);

   //若已经导入过则返回NULL_VAL
   if (VALUE_IS_NULL(result)) {
      RET_NULL;
   }

   //若编译过程中出了问题,切换到下一线程
   if (!VALUE_IS_NULL(vm->curThread->errorObj)) {
      return NO;
   }

   //回收1个slot空间
   vm->curThread->esp--;

   ObjThread* nextThread = VALUE_TO_OBJTHREAD(result);
   nextThread->caller = vm->curThread;
   vm->curThread = nextThread;
   //返回NO,vm会切换到此新加载模块的线程
   return NO;
}

//System.getModuleVariable(_,_): 获取模块args[1]中的模块变量args[2]
static bool primSystemGetModuleVariable(VM* vm, Value* args) {
   if (!validateString(vm, args[1])) {
      return NO;
   }

   if (!validateString(vm, args[2])) {
      return NO;
   }
   
   Value result = getModuleVariable(vm, args[1], args[2]);
   if (VALUE_IS_NULL(result)) {
      //出错了,给vm返回NO以切换线程
      return NO;
   }

   RET_VALUE(result);
}

//System.writeString_(_): 输出字符串args[1]
static bool primSystemWriteString(VM* vm UNUSED, Value* args) {
   ObjString* objString = VALUE_TO_OBJSTR(args[1]);
   ASSERT(objString->value.start[objString->value.length] == '\0', "string isn`t terminated!");
   printString(objString->value.start);
   RET_VALUE(args[1]);
}

//执行模块
VMResult executeModule(VM* vm, Value moduleName, const char* moduleCode) {
   ObjThread* objThread = loadModule(vm, moduleName, moduleCode);
    
   return executeInstruction(vm, objThread);
}


//编译核心模块
void buildCore(VM* vm) {

   //核心模块不需要名字,模块也允许名字为空
   ObjModule* coreModule = newObjModule(vm, NULL);

   //创建核心模块,录入到vm->allModules
   mapSet(vm, vm->allModules, CORE_MODULE, OBJ_TO_VALUE(coreModule));

   createOopFrame(vm, coreModule);
   
   //执行核心模块
   executeModule(vm, CORE_MODULE, coreModuleCode);
   
   //Bool类定义在core.script.inc中,将其挂载Bool类到vm->boolClass
   vm->boolClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Bool"));
   PRIM_METHOD_BIND(vm->boolClass, "toString", primBoolToString);
   PRIM_METHOD_BIND(vm->boolClass, "!", primBoolNot);

    bind_thread(vm, coreModule);
    
    bind_func(vm, coreModule);

    bind_overload_call(vm);
    
    bind_null(vm, coreModule);
    
    bind_number(vm, coreModule);
    
    bind_string(vm, coreModule);

   //List类
   vm->listClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "List"));
   PRIM_METHOD_BIND(vm->listClass->objHeader.class, "new()", primListNew);
   PRIM_METHOD_BIND(vm->listClass, "[_]", primListSubscript);
   PRIM_METHOD_BIND(vm->listClass, "[_]=(_)", primListSubscriptSetter);
   PRIM_METHOD_BIND(vm->listClass, "add(_)", primListAdd);
   PRIM_METHOD_BIND(vm->listClass, "addCore_(_)", primListAddCore);
   PRIM_METHOD_BIND(vm->listClass, "clear()", primListClear);
   PRIM_METHOD_BIND(vm->listClass, "count", primListCount);
   PRIM_METHOD_BIND(vm->listClass, "insert(_,_)", primListInsert);
   PRIM_METHOD_BIND(vm->listClass, "iterate(_)", primListIterate);
   PRIM_METHOD_BIND(vm->listClass, "iteratorValue(_)", primListIteratorValue);
   PRIM_METHOD_BIND(vm->listClass, "removeAt(_)", primListRemoveAt);

   //map类
   vm->mapClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Map"));
   PRIM_METHOD_BIND(vm->mapClass->objHeader.class, "new()", primMapNew);
   PRIM_METHOD_BIND(vm->mapClass, "[_]", primMapSubscript);
   PRIM_METHOD_BIND(vm->mapClass, "[_]=(_)", primMapSubscriptSetter);
   PRIM_METHOD_BIND(vm->mapClass, "addCore_(_,_)", primMapAddCore);
   PRIM_METHOD_BIND(vm->mapClass, "clear()", primMapClear);
   PRIM_METHOD_BIND(vm->mapClass, "containsKey(_)", primMapContainsKey);
   PRIM_METHOD_BIND(vm->mapClass, "count", primMapCount);
   PRIM_METHOD_BIND(vm->mapClass, "remove(_)", primMapRemove);
   PRIM_METHOD_BIND(vm->mapClass, "iterate_(_)", primMapIterate);
   PRIM_METHOD_BIND(vm->mapClass, "keyIteratorValue_(_)", primMapKeyIteratorValue);
   PRIM_METHOD_BIND(vm->mapClass, "valueIteratorValue_(_)", primMapValueIteratorValue);

   //range类
   vm->rangeClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Range"));
   PRIM_METHOD_BIND(vm->rangeClass, "from", primRangeFrom);
   PRIM_METHOD_BIND(vm->rangeClass, "to", primRangeTo);
   PRIM_METHOD_BIND(vm->rangeClass, "min", primRangeMin); 
   PRIM_METHOD_BIND(vm->rangeClass, "max", primRangeMax);
   PRIM_METHOD_BIND(vm->rangeClass, "iterate(_)", primRangeIterate);
   PRIM_METHOD_BIND(vm->rangeClass, "iteratorValue(_)", primRangeIteratorValue);

   //system类
   Class* systemClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "System"));
   PRIM_METHOD_BIND(systemClass->objHeader.class, "clock", primSystemClock);
   PRIM_METHOD_BIND(systemClass->objHeader.class, "importModule(_)", primSystemImportModule);
   PRIM_METHOD_BIND(systemClass->objHeader.class, "getModuleVariable(_,_)", primSystemGetModuleVariable);
   PRIM_METHOD_BIND(systemClass->objHeader.class, "writeString_(_)", primSystemWriteString);

   //在核心自举过程中创建了很多ObjString对象,创建过程中需要调用initObjHeader初始化对象头,
   //使其class指向vm->stringClass.但那时的vm->stringClass尚未初始化,因此现在更正.
   ObjHeader* objHeader = vm->allObjects;
   while (objHeader != NULL) {
      if (objHeader->type == OT_STRING) {
	 objHeader->class = vm->stringClass;
      }
      objHeader = objHeader->next;
   }
}
