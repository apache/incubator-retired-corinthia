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

#include "DFUnitTest.h"
#include <stddef.h>



static void test_DFextZipOpen(void)
{
#if 0
    int DFextZipOpen(const char *zipFilename, int doUnzip) {
#endif
}



static void test_DFextZipClose(void)
{
#if 0
    int DFextZipClose(void)
#endif
}



static void test_DFextZipOpenNextFile(void)
{
#if 0
    int DFextZipOpenNextFile(char *entryName, const int maxName)
#endif
}



static void test_DFextZipCloseFile(void)
{
#if 0
    int DFextZipCloseFile(void)
#endif
}



static void test_DFextZipReadCurrentFile(void)
{
#if 0
    int DFextZipReadCurrentFile(char *buf, const int maxLen)
#endif
}



static void test_DFextZipWriteCurrentFile(void)
{
#if 0
    int DFextZipWriteCurrentFile(char *buf, const int len)
#endif
}



TestGroup PlatformWrapperTests = {
    "platform.wrapper", {
            { "DFextZipOpen",            PlainTest, test_DFextZipOpen   },
            { "DFextZipClose",           PlainTest, test_DFextZipClose },
            { "DFextZipOpenNextFile",    PlainTest, test_DFextZipOpenNextFile },
            { "DFextZipCloseFile",       PlainTest, test_DFextZipCloseFile },
            { "DFextZipReadCurrentFile", PlainTest, test_DFextZipReadCurrentFile },
            { "DFextZipWriteCurrentFile", PlainTest, test_DFextZipWriteCurrentFile },
            { NULL, PlainTest, NULL }
    }
};
