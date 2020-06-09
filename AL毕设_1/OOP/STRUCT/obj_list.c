//
//  obj_list.c
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "obj_list.h"
#include "vm.h"
#include "error.h"

//新建list对象，元素个数为elementNum
ObjList *newObjList(VM *vm, uint32_t elementNum)
{
    //存储list元素的缓冲区
    Value *elementArray = NULL;
    
    //先分配内存，后调用initObjHeader,避免GC无谓的遍历
    if (elementNum > 0) {
        elementArray = ALLOCATE_ARRAY(vm, Value, elementNum);
    }
    ObjList *objList = ALLOCATE(vm, ObjList);
    
    objList->elements.datas = elementArray;
    objList->elements.capacity = objList->elements.count = elementNum;
    initObjHeader(vm, &objList->objHeader, OT_LIST, vm->listClass);
    
    return objList;
}

//在objList中索引为index处插入value,类似list[index] = value
void insertElement(VM *vm, ObjList *objList, uint32_t index, Value value)
{
    if (index > objList->elements.count - 1) {
        RUN_ERROR("index out bounded!");
    }
    
    //准备一个Value的空间以容纳新元素产生的空间波动
    //即最后一个元素要后移一个空间
    ValueBufferAdd(vm, &objList->elements, VT_TO_VALUE(VT_NULL));
    
    //下面使index后面的元素整体后移一位
    uint32_t idx = objList->elements.count - 1;
    
#warning 是否需要优化
    /*
     要是我可以插在后面，但是索引在前面就好点。
     */
    while (idx > index) {
        objList->elements.datas[idx] = objList->elements.datas[idx - 1];
        idx--;
    }
    
    //在index处插入数值
    objList->elements.datas[index] = value;
}

//调整list容量
static void shrinkList(VM *vm, ObjList *objList, uint32_t newCapacity)
{
    uint32_t oldSize = objList->elements.capacity * sizeof(Value);
    uint32_t newSize = newCapacity * sizeof(Value);
    memManager(vm, objList->elements.datas, oldSize, newSize);
    objList->elements.capacity = newCapacity;
}

//删除list中索引为index处的元素，即删除list[index]
Value removeElement(VM *vm, ObjList *objList, uint32_t index)
{
    Value valueRemoved = objList->elements.datas[index];
    
    //使index后面的元素前移一位，覆盖index处的元素
    uint32_t idx = index;
    while (idx < objList->elements.count) {
        objList->elements.datas[idx] = objList->elements.datas[idx + 1];
        idx++;
    }
    
    //若容量利用率低就减小容量（CAPACITY_GROW_FACTOR = 4，即实际元素的使用空间不足容量1/4，就缩小空间
    uint32_t _capacity = objList->elements.capacity / CAPACITY_GROW_FACTOR;
    if (_capacity > objList->elements.count) {
        shrinkList(vm, objList, _capacity);
    }
    
    objList->elements.count--;
    return valueRemoved;
}



