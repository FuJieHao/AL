//
//  error.c
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "error.h"
#include "parser.h"
#include <stdarg.h>

void errorReport(void *parser, ErrorType errorType, const char *fmt, ...)
{
    /*
     va_list,va_start,va_end
     va_list 定义一个指向参数的指针
     va_start 初始化定义的va_list变量，使其指向第一个参数
     va_arg 获取当前可变参数，返回指定类型并将指针指向下一个参数
     va_end 结束可变参数的获取
     
     vsnprintf 将可变参数格式化输出到一个字符数组
     */
    char buffer[DEFAULT_BUFFER_SIZE] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, DEFAULT_BUFFER_SIZE, fmt, ap);
    va_end(ap);
    
    switch (errorType) {
        case ERROR_IO:
            
        case ERROR_MEM:
            fprintf(stderr, "%s:%d In function %s():%s\n", __FILE__, __LINE__, __func__, buffer);
            break;
            
        case ERROR_LEX:
            
        case ERROR_COMPILE:
            
            ASSERT(&parser != NULL, "parser is NULL!");
            
            
            fprintf(stderr, "%s:%d \"%s\"\n",
                    ((Parser *)parser)->file, ((Parser *)parser)->preToken.lineNo, buffer);
            break;
            
        case ERROR_RUNTIME:
            fprintf(stderr, "%s\n",buffer);
            break;
        default:
            NOT_REACHED();
    }

    exit(1);
}

