//
//  BDTTest.h
//  dom
//
//  Created by Peter Kelly on 27/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef dfutil_BDTTest_h
#define dfutil_BDTTest_h

#include "DFBuffer.h"

void BDT_testMove(int count, int from, int to, DFBuffer *output);
void BDT_testRemove(int *indices, int count, DFBuffer *output);
int BDT_Test(int argc, const char **argv);

#endif
