//
//  cli.c
//  AL
//
//  Created by 郝富杰 on 2019/9/22.
//  Copyright © 2019 郝富杰. All rights reserved.
//


#include "test_main.h"

#include <string.h>

#include "vm.h"
#include "parser.h"
#include "core.h"


//执行脚本文件
static void runFile(const char *path)
{
    
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL) {
        char *root = (char *)malloc(lastSlash - path + 2);
        memcpy(root, path, lastSlash - path + 1);
        root[lastSlash - path + 1] = '\0';
        rootDir = root;
    }
    
    
    VM *vm = newVM();

    const char *sourceCode = readFile(path);
    
    
    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, path, (int)strlen(path))), sourceCode);
    
    
    /*
    Parser parser;
    initParser(vm, &parser, path, sourceCode, NULL);
    
    
    #include "/Users/haofujie/Desktop/AL 词法解析器调整/AL/RESOURCE/token.list"
    
    
    while (parser.curToken.type != TOKEN_EOF) {
        getNextToken(&parser);
        printf("%dL: %s [", parser.curToken.lineNo, tokenArray[parser.curToken.type]);
        uint32_t idx = 0;
        while (idx < parser.curToken.length) {
            printf("%c", *(parser.curToken.start+idx++));
        }
        printf("]\n");
    }
     */
}


int main(int argc, const char * argv[]) {

//    if (argc == 1) {
//        printf("没有指定参数程序运行结束\n");
//        ;
//    } else {
//        runFile(argv[1]);
//        printf("程序运行结束\n");
//    }
    // /Users/haofujie/Desktop/AL毕设_1/AL毕设_1/TEST/sort.al
    // /Users/haofujie/Desktop/AL毕设_1/AL毕设_1/TEST/manager.al
    //runFile("/Users/haofujie/Desktop/AL毕设_1/AL毕设_1/TEST/manager.al");
    ///Users/haofujie/Desktop/AL毕设_1/AL毕设_1/TEST/class_and_obj.al
    runFile("/Users/haofujie/Desktop/AL毕设_1/AL毕设_1/TEST/test1.al");
    
    return 0;
}





