// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "DFUnitTest.h"
#include <stddef.h>


static void test_DFGetImageDimensions(void)
{
#if 0
    int DFGetImageDimensions(const void *data, size_t len, const char *ext,
        unsigned int *width, unsigned int *height, char **errmsg)
#endif
}



static void test_DFInitOnce(void)
{
#if 0
    void DFInitOnce(DFOnce *once, DFOnceFunction fun)
#endif
}



static void test_DFMkdirIfAbsent(void)
{
#if 0
    int DFMkdirIfAbsent(const char *path, char **errmsg)
#endif
}



static void test_DFAddDirContents(void)
{
#if 0
    int DFAddDirContents(const char *absPath, const char *relPath, int recursive, DFDirEntryList ***list, char **errmsg)
#endif
}



TestGroup PlatformOSTests = {
    "platform.os", {
        { "DFGetImageDimensions", PlainTest, test_DFGetImageDimensions },
        { "DFInitOnce",           PlainTest, test_DFInitOnce },
        { "DFMkdirIfAbsent",      PlainTest, test_DFMkdirIfAbsent },
        { "DFAddDirContents",     PlainTest, test_DFAddDirContents },
        { NULL,                   PlainTest, NULL }
    }
};
