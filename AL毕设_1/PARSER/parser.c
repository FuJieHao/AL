//
//  parser.c
//  AL
//
//  Created by 郝富杰 on 2019/9/21.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "parser.h"
#include "memory.h"
#include <string.h>
#include "str_parser.h"
#include "num_parser.h"

struct keywordToken{
    char *keyword;
    uint8_t length;
    TokenType token;
}; //关键字结构

//关键字查找表
struct keywordToken keywordToken[9][5] = {
    {
        {NULL,          0,      TOKEN_UNKNOWN}
    },
    
    {},
    
    {
        {"if",          2,      TOKEN_IF},
        {"is",          2,      TOKEN_IS}
    },
    
    {
        {"var",         3,      TOKEN_VAR},
        {"fun",         3,      TOKEN_FUN},
        {"for",         3,      TOKEN_FOR}
    },
    
    {
        {"else",        4,      TOKEN_ELSE},
        {"true",        4,      TOKEN_TRUE},
        {"null",        4,      TOKEN_NULL},
        {"this",        4,      TOKEN_THIS}
    },
    
    {
        {"class",       5,      TOKEN_CLASS},
        {"false",       5,      TOKEN_FALSE},
        {"break",       5,      TOKEN_BREAK},
        {"while",       5,      TOKEN_WHILE},
        {"super",       5,      TOKEN_SUPER}
    },
    
    {
        {"return",      6,      TOKEN_RETURN},
        {"import",      6,      TOKEN_IMPORT},
        {"static",      6,      TOKEN_STATIC}
    },
    
    {},
    
    {
        {"continue",    8,      TOKEN_CONTINUE}
    }
};

//判断start是否为关键字并返回相应的token
static TokenType idOrKeyword(const char *start, uint32_t length)
{
    uint8_t idx = 0;
    while (keywordToken[length][idx].keyword != NULL) {
        if (keywordToken[length][idx].length == length &&
            memcmp(keywordToken[length][idx].keyword, start, length) == 0) {
            return keywordToken[length][idx].token;
        }
        idx++;
    }
    return TOKEN_ID;
}


//跳过连续的空白字符
static void skipBlanks(Parser *parser)
{
    //isspace判断字符是否是空白符
    //这里的空白符不单指为空格，也包括'\t''\n''\v''\f''\r'
    while (isspace(parser->curChar)) {
        if (parser->curChar == '\n') {
            parser->curToken.lineNo++;
        }
        getNextChar(parser);
    }
}

//解析标识符
static void parserId(Parser *parser, TokenType type)
{
    //判断字符是否为数字或字母
    while (isalnum(parser->curChar) || parser->curChar == '_') {
        getNextChar(parser);
    }
    
    //nextCharPtr会指向第1个不合法字符的下一个字符，因此-1
    uint32_t length = (uint32_t)(parser->nextCharPtr - parser->curToken.start - 1);

    if (type != TOKEN_UNKNOWN) {
        parser->curToken.type = type;
    } else {
        parser->curToken.type = idOrKeyword(parser->curToken.start, length);
    }
    parser->curToken.length = length;
}


//跳过一行
static void skipAline(Parser *parser)
{
    getNextChar(parser);
    while (parser->curChar != '\0') {
        if (parser->curChar == '\n') {
            parser->curToken.lineNo++;
            getNextChar(parser);
            break;
        }
        getNextChar(parser);
    }
}

//跳过行注释或区块指数
static void skipComment(Parser *parser)
{
    char nextChar = lookAheadChar(parser);
    if (parser->curChar == '/') {  //行注释
        skipAline(parser);
    } else {
        //区块注释
        while (nextChar != '*' && nextChar != '\0') {
            getNextChar(parser);
            if (parser->curChar == '\n') {
                parser->curToken.lineNo++;
            }
            nextChar = lookAheadChar(parser);
        }
        if (matchNextChar(parser, '*')) {
            if (!matchNextChar(parser, '/')) {  //匹配*/
                LEX_ERROR(parser, "expect '/' after '*'!");
            }
            getNextChar(parser);
        } else {
            LEX_ERROR(parser, "expect '*/' before file end!");
        }
    }
    skipBlanks(parser);     //注释之后有可能会有空白字符
}

//获取下一个token
void getNextToken(Parser *parser)
{
    parser->preToken = parser->curToken;
    skipBlanks(parser);     //跳过待识别单词之前的空格
    parser->curToken.type = TOKEN_EOF;
    parser->curToken.length = 0;
    parser->curToken.start = parser->nextCharPtr - 1;
    parser->curToken.value = VT_TO_VALUE(VT_UNDEFINED);
    while (parser->curChar != '\0') {
        switch (parser->curChar) {
            case ',':
                parser->curToken.type = TOKEN_COMMA;
                break;
            case ':':
                parser->curToken.type = TOKEN_COLON;
                break;
            case '(':
                if (parser->interpolationExpectRightParenNum > 0) {
                    parser->interpolationExpectRightParenNum++;
                }
                parser->curToken.type = TOKEN_LEFT_PAREN;
                break;
            case ')':
                if (parser->interpolationExpectRightParenNum > 0) {
                    parser->interpolationExpectRightParenNum--;
                    if (parser->interpolationExpectRightParenNum == 0) {
                        parserString(parser);
                        break;
                    }
                }
                parser->curToken.type = TOKEN_RIGHT_PAREN;
                break;
            case '[':
                parser->curToken.type = TOKEN_LEFT_BRACKET;
                break;
            case ']':
                parser->curToken.type = TOKEN_RIGHT_BRACKET;
                break;
            case '{':
                parser->curToken.type = TOKEN_LEFT_BRACE;
                break;
            case '}':
                parser->curToken.type = TOKEN_RIGHT_BRACE;
                break;
            case '.':
                if (matchNextChar(parser, '.')) {
                    parser->curToken.type = TOKEN_DOT_DOT;
                } else {
                    parser->curToken.type = TOKEN_DOT;
                }
                break;
            case '=':
                if (matchNextChar(parser, '=')) {
                    parser->curToken.type = TOKEN_EQUAL;
                } else {
                    parser->curToken.type = TOKEN_ASSIGN;
                }
                break;
            case '+':
                parser->curToken.type = TOKEN_ADD;
                break;
            case '-':
                parser->curToken.type = TOKEN_SUB;
                break;
            case '*':
                parser->curToken.type = TOKEN_MUL;
                break;
            case '/':
                //跳过注释'//'或'/*'
                if (matchNextChar(parser, '/') || matchNextChar(parser, '*')) {
                    skipComment(parser);
                    
                    //重置下一个token起始位置
                    parser->curToken.start = parser->nextCharPtr - 1;
                    
                    continue;
                } else {
                    parser->curToken.type = TOKEN_DIV;
                }
                break;
            case '%':
                parser->curToken.type = TOKEN_MOD;
                break;
            case '&':
                if (matchNextChar(parser, '&')) {
                    parser->curToken.type = TOKEN_LOGIC_AND;
                } else {
                    parser->curToken.type = TOKEN_BIT_AND;
                }
                break;
            case '|':
                if (matchNextChar(parser, '|')) {
                    parser->curToken.type = TOKEN_LOGIC_OR;
                } else {
                    parser->curToken.type = TOKEN_BIT_OR;
                }
                break;
            case '~':
                parser->curToken.type = TOKEN_BIT_NOT;
                break;
            case '?':
                parser->curToken.type = TOKEN_QUESTION;
                break;
            case '>':
                if (matchNextChar(parser, '=')) {
                    parser->curToken.type = TOKEN_GREATE_EQUAL;
                } else if (matchNextChar(parser, '>')) {
                    parser->curToken.type = TOKEN_BIT_SHIFT_RIGHT;
                } else {
                    parser->curToken.type = TOKEN_GREATE;
                }
                break;
            case '<':
                if (matchNextChar(parser, '=')) {
                    parser->curToken.type = TOKEN_LESS_EQUAL;
                } else if (matchNextChar(parser, '<')) {
                    parser->curToken.type = TOKEN_BIT_SHIFT_LEFT;
                } else {
                    parser->curToken.type = TOKEN_LESS;
                }
                break;
            case '!':
                if (matchNextChar(parser, '=')) {
                    parser->curToken.type = TOKEN_NOT_EQUAL;
                } else {
                    parser->curToken.type = TOKEN_LOGIC_NOT;
                }
                break;
            case '"':
                parserString(parser);
                break;
            default:
                //处理变量名及数字
                //进入此分支的字符肯定是数字或变量名的首字符
                //后面会调用相应的函数把其余字符一并解析
                //不过识别数字需要一些依赖，目前暂时去掉
                
                //若首字符是字母或"_"则是变量名
                if (isalpha(parser->curChar) || parser->curChar == '_') {
                    parserId(parser, TOKEN_UNKNOWN); //解析变量名其余的部分
                } else if (isdigit(parser->curChar)) { //数字
                    parseNum(parser);
                } else {
                    if (parser->curChar == '#' && matchNextChar(parser, '!')) {
                        skipAline(parser);
                        parser->curToken.start = parser->nextCharPtr - 1;
                        //重置下一个token起始位置
                        continue;
                    }
                    LEX_ERROR(parser, "unsupport char: \'%c\', quit.", parser->curChar);
                }
                return;
        }
        //大部分case的出口
        parser->curToken.length = (uint32_t)(parser->nextCharPtr - parser->curToken.start);
        getNextChar(parser);
        return;
    }
}


//若当前token为expected则读入下一个token并返回YES, 若不读入token返回NO
bool matchToken(Parser *parser, TokenType expected)
{
    if (parser->curToken.type == expected) {
        getNextToken(parser);
        return YES;
    }
    return NO;
}
//断言当前token为expected并读入下一个token，否则报错errMsg
void consumeCurToken(Parser *parser, TokenType expected, const char *errMsg)
{
    if (parser->curToken.type != expected) {
        COMPILE_ERROR(parser, errMsg);
    }
    getNextToken(parser);
}
//断言下一个token为expected，否则报错errMsg
void consumeNextToken(Parser *parser, TokenType expected, const char *errMsg)
{
    getNextToken(parser);
    if (parser->curToken.type != expected) {
        COMPILE_ERROR(parser, errMsg);
    }
}
//由于sourceCode未必来自于文件file,有可能只是个字符串
//file仅用作跟踪待编译的代码的标识，方便报错
void initParser(VM *vm, Parser *parser, const char *file, const char *sourceCode, ObjModule *objModule)
{
    parser->file = file;
    parser->sourceCode = sourceCode;
    parser->curChar = *parser->sourceCode;
    parser->nextCharPtr = parser->sourceCode + 1;
    parser->curToken.lineNo = 1;
    parser->curToken.type = TOKEN_UNKNOWN;
    parser->curToken.start = NULL;
    parser->curToken.length = 0;
    parser->preToken = parser->curToken;
    parser->interpolationExpectRightParenNum = 0;
    parser->vm = vm;
    parser->curModule = objModule;
}

//uint32_t getByteNumOfEncodeUtf8(int value);
//uint8_t encodeUtf8(uint8_t *buf, int value);

