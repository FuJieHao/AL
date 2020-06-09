//
//  debug.h
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef debug_h
#define debug_h

/*do{}while(0)程序只执行一次，既然如此，为什么还要把判断包进去？
 1.把内部代码当成一个{}处理 2.用do while + break可以实现goto的效果
 ...（还有一些其余的用处）
 */
/* stderr : 将错误信息送到标准错误文件 ，在C的程序执行中，一直处于开启状态*/
#ifdef DEBUG
    #define ASSERT(condition, errMsg) \
        do {\
            if (!(condition)) {\
                fprintf(stderr, "ASSERT failed! %s:%d In function %s(): %s\n",\
                __FILE__,__LINE__,__func__,errMsg);\
                abort();\
            }\
        } while (0);
#else
/*(void)0 和 NULL的区别：
 NULL等于(void *)0是个空指针, (void)0不可以赋值给其它类型，也不可以进行算数运算，逻辑运算
 这里(void)0显然更符合需求*/
    #define ASSERT(condition, errMsg) ((void)0)
#endif

/*不可达处理 while(1)将程序卡在这部分*/
#define NOT_REACHED() \
    do {\
        fprintf(stderr, "NOT_REACHED: %s:%d In function %s()\n",\
        __FILE__,__LINE__,__func__);\
        while(1);\
    } while (0);


#endif /* debug_h */
