//
//  parser.h
//  AL
//
//  Created by 郝富杰 on 2019/9/21.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include "parser_common.h"
#include "vm.h"

#define PEEK_TOKEN(parserPtr) parserPtr->curToken.type

char lookAheadChar(Parser *parser);
void getNextToken(Parser *parser);
bool matchToken(Parser *parser, TokenType expected);
void consumeCurToken(Parser *parser, TokenType expected, const char *errMsg);
void consumeNextToken(Parser *parser, TokenType expected, const char *errMsg);
//uint32_t getByteNumOfEncodeUtf8(int value);
//uint8_t encodeUtf8(uint8_t *buf, int value);
void initParser(VM *vm, Parser *parser, const char *file, const char *sourceCode, ObjModule *objModule);
#endif /* parser_h */









