//
//  error.h
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef error_h
#define error_h

#include "common.h"

// 错误类型结构体
typedef enum {
    ERROR_IO,       //文件IO错误
    ERROR_MEM,      //内存分配错误
    ERROR_LEX,      //词法分析错误
    ERROR_COMPILE,  //编译错误
    ERROR_RUNTIME   //运行时错误
} ErrorType;


// 错误报告函数
void errorReport(void *parser, ErrorType errorType, const char *fmt, ...);

// __VA_ARGS__ 函数用于宏中，将函数参数用__VA_ARGS__替代，相当于...的功能。

#define IO_ERROR(...) \
    errorReport(NULL, ERROR_IO, __VA_ARGS__)

#define MEM_ERROR(...) \
    errorReport(NULL, ERROR_MEM, __VA_ARGS__)

#define LEX_ERROR(parser, ...) \
    errorReport(parser, ERROR_LEX, __VA_ARGS__)

#define COMPILE_ERROR(parser, ...) \
    errorReport(NULL, ERROR_COMPILE, __VA_ARGS__)

#define RUN_ERROR(...) \
    errorReport(NULL, ERROR_RUNTIME, __VA_ARGS__)

#endif /* error_h */
