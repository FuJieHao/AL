//
//  parser_common.c
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "parser_common.h"

//向后取出一个字符
char lookAheadChar(Parser *parser)
{
    return *parser->nextCharPtr;
}
//得到并指向下一个字符
void getNextChar(Parser *parser)
{
    parser->curChar = *parser->nextCharPtr++;
}
//查看下一个字符是否为期望的，如果是就读进来，返回YES,否则NO
bool matchNextChar(Parser *parser, char expectedChar)
{
    if (lookAheadChar(parser) == expectedChar) {
        getNextChar(parser);
        return YES;
    }
    return NO;
}
