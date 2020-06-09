//
//  str_parser.c
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#include "str_parser.h"

//解析unicode码点
static void parserUnicodeCodePoint(Parser *parser, ByteBuffer *buf)
{
    uint32_t idx = 0;
    int value = 0;
    uint8_t digit = 0;
    
    //获取数值，u后面跟着4位十六进制数字
    while (idx++ < 4) {
        getNextChar(parser);
        if (parser->curChar == '\0') {
            LEX_ERROR(parser, "unterminated unicode!");
        }
        if (parser->curChar >= '0' && parser->curChar <= '9') {
            digit = parser->curChar - '0';
        } else if (parser->curChar >= 'a' && parser->curChar <= 'f') {
            digit = parser->curChar - 'a' + 10;
        } else if (parser->curChar >= 'A' && parser->curChar <= 'F') {
            digit = parser->curChar - 'A' + 10;
        } else {
            LEX_ERROR(parser, "invalia unicode!");
        }
        value = value * 16 | digit;
    }
    
    uint32_t byteNum = getByteNumOfEncodeUtf8(value);
    ASSERT(byteNum != 0, "utf8 encode bytes should be between 1 and 4!");
    
    //为代码通用，下面会直接写buf->datas,在此先写入byteNum个0，以保证事先有byteNum个空间
    ByteBufferFillWrite(parser->vm, buf, 0, byteNum);
    
    //把value编码为utf-8后写入缓冲区buf
    encodeUtf8(buf->datas + buf->count - byteNum, value);
}

//解析字符串
void parserString(Parser *parser)
{
    ByteBuffer str;
    ByteBufferInit(&str);
    
    while (YES) {
        getNextChar(parser);
        
        if (parser->curChar == '\0') {
            LEX_ERROR(parser, "unterminated string!");
        }
        
        if (parser->curChar == '"') {
            parser->curToken.type = TOKEN_STRING;
            break;
        }
        
        if (parser->curChar == '%') {
            if (!matchNextChar(parser, '(')) {
                LEX_ERROR(parser, "'%' should followed by '('!");
            }
            if (parser->interpolationExpectRightParenNum > 0) {
                COMPILE_ERROR(parser,"sorry, I don't support nest interpolate expression!");
            }
            parser->interpolationExpectRightParenNum = 1;
            parser->curToken.type = TOKEN_INTERPILATION;
            break;
        }
        if (parser->curChar == '\\') {     //处理转义字符
            getNextChar(parser);
            switch (parser->curChar) {
                case '0':
                    ByteBufferAdd(parser->vm, &str, '\0');
                    break;
                case 'a':
                    ByteBufferAdd(parser->vm, &str, '\a');
                    break;
                case 'b':
                    ByteBufferAdd(parser->vm, &str, '\b');
                    break;
                case 'f':
                    ByteBufferAdd(parser->vm, &str, '\f');
                    break;
                case 'n':
                    ByteBufferAdd(parser->vm, &str, '\n');
                    break;
                case 'r':
                    ByteBufferAdd(parser->vm, &str, '\r');
                    break;
                case 't':
                    ByteBufferAdd(parser->vm, &str, '\t');
                    break;
                case 'u':
                    parserUnicodeCodePoint(parser, &str);
                    break;
                case '"':
                    ByteBufferAdd(parser->vm, &str, '"');
                    break;
                case '\\':
                    ByteBufferAdd(parser->vm, &str, '\\');
                default:
                    LEX_ERROR(parser,"unsupport escape \\%c", parser->curChar);
                    break;
            }
        } else {    //普通字符
            ByteBufferAdd(parser->vm, &str, parser->curChar);
        }
    }
    
    //用识别到的字符串新建字符串对象存储到curToken的value中
    ObjString *objString = newObjString(parser->vm, (const char *)str.datas, str.count);
    parser->curToken.value = OBJ_TO_VALUE(objString);
    ByteBufferClear(parser->vm, &str);
}
