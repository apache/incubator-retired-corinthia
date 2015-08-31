// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "Common.h"
#include "Util.h"
#include <stdio.h>

#define isstart1(c) (((c) & 0x80) == 0x00) // 0xxxxxxx
#define isstart2(c) (((c) & 0xE0) == 0xC0) // 110xxxxx
#define isstart3(c) (((c) & 0xF0) == 0xE0) // 1110xxxx
#define isstart4(c) (((c) & 0xF8) == 0xF0) // 11110xxx
#define iscont(c)   (((c) & 0xC0) == 0x80) // 10xxxxxx

uint32_t UTF8NextChar(const char *str, size_t *offsetp)
{
    size_t pos = *offsetp;
    const uint8_t *input = (const uint8_t *)str;

    // If we're part-way through a multi-byte character, skip to the beginning of the next one
    while ((input[pos] != 0) && iscont(input[pos]))
        pos++;

    // End of string
    if (input[pos] == 0) {
        *offsetp = pos;
        return 0;
    }

    if (isstart1(input[pos])) {
        // 1-byte character: 0xxxxxxx
        uint32_t a = input[pos];
        *offsetp = pos+1;
        return a;
    }
    else if (isstart2(input[pos]) && iscont(input[pos+1])) {
        // 2-byte character: 110xxxxx 10xxxxxx
        uint32_t a = input[pos+1] & 0x3F; // a = bits 0-5
        uint32_t b = input[pos+0] & 0x1F; // b = bits 6-10
        *offsetp = pos+2;
        return (a << 0) | (b << 6);
    }
    else if (isstart3(input[pos]) && iscont(input[pos+1]) && iscont(input[pos+2])) {
        // 3-byte character: 1110xxxx 10xxxxxx 10xxxxxx
        uint32_t a = input[pos+2] & 0x3F; // a = bits 0-5
        uint32_t b = input[pos+1] & 0x3F; // b = bits 6-11
        uint32_t c = input[pos+0] & 0x0F; // c = bits 12-15
        *offsetp = pos+3;
        return (a << 0) | (b << 6) | (c << 12);
    }
    else if (isstart4(input[pos]) && iscont(input[pos+1]) && iscont(input[pos+2]) && iscont(input[pos+3])) {
        // 4-byte character: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        uint32_t a = input[pos+3] & 0x3F; // a = bits 0-5
        uint32_t b = input[pos+2] & 0x3F; // b = bits 6-11
        uint32_t c = input[pos+1] & 0x3F; // c = bits 12-17
        uint32_t d = input[pos+0] & 0x07; // d = bits 18-20
        *offsetp = pos+4;
        return (a << 0) | (b << 6) | (c << 12) | (d << 18);
    }
    else {
        // Invalid UTF8 byte sequence
        *offsetp = pos;
        return 0;
    }
}

int printEscapedRangeChar(char c)
{
    int chars = 0;
    switch (c) {
        case '[':
            chars += printf("\\[");
            break;
        case ']':
            chars += printf("\\]");
            break;
        case '\\':
            chars += printf("\\\\");
            break;
        default:
            chars += printf("%c",c);
            break;
    }
    return chars;
}

int printLiteral(const char *value)
{
    int chars = 0;
    chars += printf("\"");
    for (int i = 0; value[i] != '\0'; i++) {
        switch (value[i]) {
            case '\r':
                chars += printf("\\r");
                break;
            case '\n':
                chars += printf("\\n");
                break;
            case '\t':
                chars += printf("\\t");
                break;
            case '\"':
                chars += printf("\\\"");
                break;
            case '\\':
                chars += printf("\\\\");
                break;
            default:
                chars += printf("%c",value[i]);
        }
    }
    chars += printf("\"");
    return chars;
}
