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

#ifndef dfutil_Test_h
#define dfutil_Test_h

#include "DFArray.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           TestHarness                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    int showResults;
    int showDiffs;
    int passed;
    int failed;
} TestHarness;

void TestRun(TestHarness *harness, const char *path);

void TestGetFilenamesRecursive(const char *path, DFArray *result);

#endif
