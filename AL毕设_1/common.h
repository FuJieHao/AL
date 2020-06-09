//
//  common.h
//  AL
//
//  Created by 郝富杰 on 2019/9/19.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef common_h
#define common_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "error.h"
#include "debug.h"

/// 虚拟机
typedef struct vm VM;
/// 词法分析器
typedef struct parser Parser;
/// 编译单元
typedef struct compileUnit CompileUnit;
typedef struct class Class;

#define bool char
#define YES 1
#define NO 0

//表示作用域中最大的局部变量个数是128
#define MAX_LOCAL_AVR_NUM 128
//表示最大的upvalue的个数是128
#define MAX_UPVALUE_NUM 128
#define MAX_ID_LEN 128  //变量名的最大长度

//表示方法名的最大长度是128
#define MAX_METHOD_NAME_LEN MAX_ID_LEN
//表示最多的参数是16个
#define MAX_ARG_NUM 16

//方法签名 = 函数名长度+'('+n个参数+(n-1)个参数分隔符','+')'
#define MAX_SIGN_LEN MAX_METHOD_NAME_LEN + MAX_ARG_NUM * 2 + 1

//表示类中属性的最大数量为128
#define MAX_FIELD_NUM 128

/*atttribute 可以设置函数属性（function attribute）、变量属性(variable attribute)和类型属性（type attribute） attribute((unused)) (function attribute). attribute((unused)) 其作用是即使没有使用这个函数，编译器也不警告。*/
#define UNUSED __attribute__((unused))


#endif /* common_h */
