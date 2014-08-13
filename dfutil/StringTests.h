//
//  StringTests.h
//  dfutil
//
//  Created by Peter Kelly on 18/07/2014.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef dfutil_StringTests_h
#define dfutil_StringTests_h

#include <stdint.h>

#ifdef __APPLE__

int testDFNextChar(const uint32_t *input32, size_t len32);
int testDFPrevChar(const uint32_t *input32, size_t len32);
uint32_t *genTestUTF32String(size_t len32);
int testUnicode(void);
int testStrings(void);

#endif

#endif
