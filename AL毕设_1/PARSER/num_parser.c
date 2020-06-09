//
//  num_parser.c
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "num_parser.h"

//解析十六进制数字
static void parseHexNum(Parser *parser)
{
    while (isxdigit(parser->curChar)) {
        getNextChar(parser);
    }
}

//解析十进制数字
static void parseDecNum(Parser *parser)
{
    while (isdigit(parser->curChar)) {
        getNextChar(parser);
    }
    
    //若有小数点
    if (parser->curChar == '.' && isdigit(lookAheadChar(parser))) {
        getNextChar(parser);
        while (isdigit(parser->curChar)) { //解析小数点之后的数字
            getNextChar(parser);
        }
    }
}

//解析八进制数字
static void parseOctNum(Parser *parser)
{
    while (parser->curChar >= '0' && parser->curChar < '8') {
        getNextChar(parser);
    }
}

//解析八进制、十进制、十六进制，仅支持前缀形式，后缀形式不支持
void parseNum(Parser *parser)
{
    //十六进制的0x前缀
    if (parser->curChar == '0' && matchNextChar(parser, 'x')) {
        getNextChar(parser);    //跳过x
        parseHexNum(parser);    //解析十六进制数字
        parser->curToken.value =
        NUM_TO_VALUE(strtol(parser->curToken.start, NULL, 16));
    } else if (parser->curChar == '0' && isdigit(lookAheadChar(parser))) {
        //八进制
        parseOctNum(parser);
        parser->curToken.value =
        NUM_TO_VALUE(strtol(parser->curToken.start, NULL, 8));
    } else {
        parseDecNum(parser);
        parser->curToken.value = NUM_TO_VALUE(strtod(parser->curToken.start, NULL));
    }
    
    //nextCharPtr会指向第一个不合法字符的下一个字符，因此-1
    parser->curToken.length =
    (uint32_t)(parser->nextCharPtr - parser->curToken.start - 1);
    parser->curToken.type = TOKEN_NUM;
}

