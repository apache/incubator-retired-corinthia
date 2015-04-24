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

#import <Foundation/Foundation.h>
#include "DFPlatform.h"
#include "StringTests.h"
#include "DFString.h"

int testDFNextChar(const uint32_t *input32, size_t len32)
{
    printf("Testing DFNextChar\n");
    NSString *ns = [[NSString alloc] initWithBytes: input32 length: len32*sizeof(uint32_t)
                                          encoding: NSUTF32LittleEndianStringEncoding];
    assert(ns != nil);

    int widthHistogram[5];
    bzero(widthHistogram,5*sizeof(int));

    const char *eight = ns.UTF8String;
    size_t pos8 = 0;
    size_t pos32 = 0;
    int errors = 0;
    for (;;) {
        size_t oldpos8 = pos8;
        uint32_t ch = DFNextChar(eight,&pos8);
        ssize_t nbytes = pos8 - oldpos8;
        assert((pos32 == len32) || (nbytes > 0));
        if (pos32 < len32) {
            assert(nbytes >= 1);
            assert(nbytes <= 4);
            widthHistogram[nbytes]++;
        }
        if (ch != input32[pos32])
            errors++;
        if (pos32 == len32)
            break;
        pos32++;
    }

    for (int i = 1; i <= 4; i++) {
        printf("    Characters requiring %d bytes: %d\n",i,widthHistogram[i]);
    }
    printf("    Total characters: %lu\n",len32);
    printf("    Total bytes: %lu\n",strlen(eight));
    printf("    Errors: %d\n",errors);
    return errors;
}

int testDFPrevChar(const uint32_t *input32, size_t len32)
{
    printf("Testing DFPrevChar\n");
    assert(len32 > 0);
    NSString *ns = [[NSString alloc] initWithBytes: input32 length: len32*sizeof(uint32_t)
                                          encoding: NSUTF32LittleEndianStringEncoding];
    assert(ns != nil);

    int widthHistogram[5];
    bzero(widthHistogram,5*sizeof(int));

    const char *eight = ns.UTF8String;
    assert(strlen(eight) > 0);
    size_t pos8 = strlen(eight) - 1;
    size_t pos32 = len32-1;
    int errors = 0;
    for (;;) {
        size_t oldpos8 = pos8;
        uint32_t ch = DFPrevChar(eight,&pos8);
        ssize_t nbytes = oldpos8 - pos8;
        assert((pos32 == len32) || (nbytes > 0));
        if (pos32 < len32) {
            assert(nbytes >= 1);
            assert(nbytes <= 4);
            widthHistogram[nbytes]++;
        }
        if (ch != input32[pos32])
            errors++;
        if (pos32 == 0)
            break;
        pos32--;
    }

    for (int i = 1; i <= 4; i++) {
        printf("    Characters requiring %d bytes: %d\n",i,widthHistogram[i]);
    }
    printf("    Total characters: %lu\n",len32);
    printf("    Total bytes: %lu\n",strlen(eight));
    printf("    Errors: %d\n",errors);
    return errors;
}

uint32_t *genTestUTF32String(size_t len32)
{
    uint32_t *str32 = (uint32_t *)malloc((len32+1)*sizeof(uint32_t));
    for (size_t i = 0; i < len32; i++) {
        do {
            uint32_t min = 0;
            uint32_t max = 0;

            int nbytes = 1 + (rand()%4);
            switch (nbytes) {
                case 1:
                    min = 0;
                    max = 0x7F;
                    break;
                case 2:
                    min = 0x80;
                    max = 0x7FF;
                    break;
                case 3:
                    min = 0x800;
                    max = 0xFFFF;
                    break;
                case 4:
                    min = 0x10000;
                    max = 0x10FFFF;
                    break;
                default:
                    assert(0);
                    break;
            }

            assert(max > 0);

            str32[i] = min + rand()%(max - min);

            assert(str32[i] < 0x10FFFF);
        } while ((str32[i] == 0) || ((str32[i] >= 0xD800) && (str32[i] <= 0xDFFF)));
    }
    str32[len32] = 0;
    return str32;
}

int testUnicode(void)
{
    int pass = 1;
    uint32_t value;
    uint32_t array[2];
    for (value = 1; value <= 0x10FFFF; value++) {

        if ((value >= 0xD800) && (value <= 0xDFFF))
            continue;

        @autoreleasepool {
            array[0] = value;
            array[1] = 0;

            NSString *str = [[NSString alloc] initWithBytes: &value length: 4 encoding: NSUTF32LittleEndianStringEncoding];
            const char *nsutf8 = str.UTF8String;
            char *myutf8 = DFUTF32to8(array);
            if (strcmp(nsutf8,myutf8)) {
                printf("value = 0x%X FAIL 32 to 8\n",value);
                pass = 0;
            }
            else {
                uint32_t *myutf32 = DFUTF8To32(myutf8);
                int mylen = 0;
                while (myutf32[mylen] != 0)
                    mylen++;
                if ((mylen != 1) || (myutf32[0] != value)) {
                    printf("value = 0x%X FAIL 8 to 32\n",value);
                    pass = 0;
                }
                else {
                    printf("value = 0x%X PASS\n",value);
                }
            }
        }
    }
    return pass;
}

int testStrings(void)
{
    size_t len32 = 1024*1024;
    uint32_t *str32 = genTestUTF32String(len32);
    testDFNextChar(str32,len32);
    testDFPrevChar(str32,len32);
    return 1;
}
