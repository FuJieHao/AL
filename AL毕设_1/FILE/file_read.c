//
//  core.c
//  AL
//
//  Created by 郝富杰 on 2019/9/21.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "file_read.h"
#include <string.h>
#include <sys/stat.h>
#include "vm.h"

char *rootDir = NULL;


char *readFile(const char *path)
{
    //
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        IO_ERROR("Could not open file \"%s\".\n",path);
    }
    
    //在结构体调用函数stat 获取path路径的文件的相关属性(大小，创建，访问时间)
    struct stat fileStat;
    stat(path, &fileStat);
    
    size_t fileSize = fileStat.st_size;
    
    char *fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL) {
        MEM_ERROR("Could not allcate memory for reading file \"%s\".\n", path);
    }
    
    size_t numRead = fread(fileContent, sizeof(char), fileSize, file);
    if (numRead < fileSize) {
        IO_ERROR("Could not read file \"%s\".\n", path);
    }
    
    fileContent[fileSize] = '\0';
    fclose(file);
    
    return fileContent;
    
}
