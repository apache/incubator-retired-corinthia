//
//  Test.h
//  dom
//
//  Created by Peter Kelly on 27/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

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
