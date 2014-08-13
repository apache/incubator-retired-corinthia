//
//  TestCase.h
//  dfutil
//
//  Created by Peter Kelly on 27/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef dfutil_TestCase_h
#define dfutil_TestCase_h

#include "DFHashTable.h"
#include "WordPackage.h"

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
    char *tempPath;
    char *concretePath;
    char *abstractPath;
};

TestCase *TestCaseNew(const char *path, DFHashTable *input);
void TestCaseFree(TestCase *tc);
int TestCaseGetWordPackage(TestCase *tc, WordPackage *package);
DFDocument *TestCaseGetHTML(TestCase *script);

#endif
