//
//  unicodeUtf8.h
//  AL
//
//  Created by 郝富杰 on 2019/9/21.
//  Copyright © 2019 郝富杰. All rights reserved.
//

#ifndef unicodeUtf8_h
#define unicodeUtf8_h

#include <stdint.h>

/// 获得传入数字value在utf-8编码下需要的字节数
uint8_t getByteNumOfEncodeUtf8(int value);
/// 把value编码为utf-8后写入缓冲区Buf,返回写入的字节数
uint8_t encodeUtf8(uint8_t *buf, int value);
/// 返回解码UTF-8的字节数
uint8_t getByteNumOfDecodeUtf8(uint8_t byte);
/// 将数字从缓冲区中解码取出
int decodeUtf8(const uint8_t *bytePtr, uint32_t length);

#endif /* unicodeUtf8_h */
