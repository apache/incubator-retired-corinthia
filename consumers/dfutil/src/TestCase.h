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

#ifndef dfutil_TestCase_h
#define dfutil_TestCase_h

#include "DFHashTable.h"
#include "DFBuffer.h"
#include "DFPackage.h"
#include "DFXMLForward.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            TestCase                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct TestCase TestCase;

struct TestCase {
    char *path;
    DFHashTable *input;
    DFBuffer *output;
};

TestCase *TestCaseNew(const char *path, DFHashTable *input);
void TestCaseFree(TestCase *tc);
DFPackage *TestCaseOpenPackage(TestCase *tc, DFError **error);
DFDocument *TestCaseGetHTML(TestCase *tc, DFPackage *htmlPackage, DFError **error);

#endif
