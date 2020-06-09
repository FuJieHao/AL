//
//  parser_common.h
//  AL
//
//  Created by 郝富杰 on 2019/9/28.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef parser_common_h
#define parser_common_h

#include "common.h"
#include "unicodeUtf8.h"
#include "class.h"
#include "meta_obj.h"
#include <ctype.h>
#include "compiler.h"

typedef enum {
    TOKEN_UNKNOWN,
    
    //数据类型
    TOKEN_NUM,              //数字
    TOKEN_STRING,           //字符串
    TOKEN_ID,               //变量名
    TOKEN_INTERPILATION,    //内嵌表达式
    
    //关键字（系统保留字)
    TOKEN_VAR,              //var
    TOKEN_FUN,              //fun
    TOKEN_IF,               //if
    TOKEN_ELSE,             //else
    TOKEN_TRUE,             //TRUE
    TOKEN_FALSE,            //NO
    TOKEN_WHILE,            //while
    TOKEN_FOR,              //for
    TOKEN_BREAK,            //break
    TOKEN_CONTINUE,         //continue
    TOKEN_RETURN,           //return
    TOKEN_NULL,             //null
    
    //类和模块导入的token
    TOKEN_CLASS,            //class
    TOKEN_THIS,             //this
    TOKEN_STATIC,           //static
    TOKEN_IS,               //is
    TOKEN_SUPER,            //super
    TOKEN_IMPORT,           //import
    
    //分隔符
    TOKEN_COMMA,            //,
    TOKEN_COLON,            //:
    TOKEN_LEFT_PAREN,       //(
    TOKEN_RIGHT_PAREN,      //)
    TOKEN_LEFT_BRACKET,     //[
    TOKEN_RIGHT_BRACKET,    //]
    TOKEN_LEFT_BRACE,       //{
    TOKEN_RIGHT_BRACE,      //}
    TOKEN_DOT,              //.
    TOKEN_DOT_DOT,          //..
    
    //简单双目运算符
    TOKEN_ADD,              //+
    TOKEN_SUB,              //-
    TOKEN_MUL,              //*
    TOKEN_DIV,              ///
    TOKEN_MOD,              //%
    
    //赋值运算符
    TOKEN_ASSIGN,           //=
    
    //位运算符
    TOKEN_BIT_AND,          //&
    TOKEN_BIT_OR,           //|
    TOKEN_BIT_NOT,          //~
    TOKEN_BIT_SHIFT_RIGHT,  //>>
    TOKEN_BIT_SHIFT_LEFT,   // <<
    
    //逻辑运算符
    TOKEN_LOGIC_AND,        //&&
    TOKEN_LOGIC_OR,         //||
    TOKEN_LOGIC_NOT,        //!
    
    //关系操作符
    TOKEN_EQUAL,            //==
    TOKEN_NOT_EQUAL,        //!=
    TOKEN_GREATE,           //>
    TOKEN_GREATE_EQUAL,     //>=
    TOKEN_LESS,             // <
    TOKEN_LESS_EQUAL,       // <=
    
    TOKEN_QUESTION,         //?
    
    //文件结束标记，仅词法分析使用
    TOKEN_EOF
    
}TokenType;

// 用以存储经词法分析器分析后得到的lex结构 ，供语法分析器使用
typedef struct {
    TokenType type;     //类型
    const char *start;  //指向源码中单词的起始地址
    uint32_t length;    //长度
    uint32_t lineNo;    //所在源码的行号
    Value value;        //这里的值是内容的意思，不是数值
}Token;

struct parser {
    const char *file;           //源码文件名
    const char *sourceCode;     //指向源码字符串
    const char *nextCharPtr;    //指向sourceCode中下一个字符
    char curChar;              //识别当前字符
    Token curToken;            //当前token
    Token preToken;             //前一个token
    ObjModule *curModule;      //当前正在编译的模块
    
    /*词法分析器在任意时刻总会处于一个编译单元中，模块、方法、函数都有自己的编译单元*/
    /*模块也是编译单元的原因是变量可直接定义在模块中，即模块变量*/
    /*类没有编译单元，因为编译单元是独立的指令流单位，类是由方法组成的，方法才是指令流的单位。*/
    CompileUnit *curCompileUnit;       //当前编译的单元
    
    //处于内嵌表达式之中，期望的右括号数量
    //用于跟踪小括号对的嵌套
    int interpolationExpectRightParenNum;
    struct parser *parent;      //指向父parser
    
    VM *vm;
};

/// 向后取出一个字符
char lookAheadChar(Parser *parser);
/// 得到并指向下一个字符
void getNextChar(Parser *parser);
/// 查看下一个字符是否为期望的，如果是就读进来，返回YES,否则NO
bool matchNextChar(Parser *parser, char expectedChar);

#endif /* parser_common_h */
