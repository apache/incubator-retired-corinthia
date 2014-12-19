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

#include "TestFunctions.h"
#include "DFBDT.h"
#include "BDTTests.h"
#include "WordPlain.h"
#include "HTMLPlain.h"
#include "Commands.h"
#include "DFChanges.h"
#include "WordConverter.h"
#include "Word.h"
#include "DFHTML.h"
#include "DFHTMLNormalization.h"
#include "DFFilesystem.h"
#include "DFString.h"
#include "HTMLToLaTeX.h"
#include "DFXML.h"
#include "DFCommon.h"
#include "DFUnitTest.h"
#include <DocFormats/DocFormats.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern TestGroup APITests;
extern TestGroup CSSTests;
extern TestGroup HTMLTests;
extern TestGroup LibTests;
extern TestGroup XMLTests;
extern TestGroup LaTeXTests;
extern TestGroup ODFTests;
extern TestGroup WordTests;
extern TestGroup PlatformTests;
extern TestGroup BDTTests;

TestGroup *allGroups[] = {
    &APITests,
    &BDTTests,
    &CSSTests,
    &HTMLTests,
    &LibTests,
    &XMLTests,
    &LaTeXTests,
    &ODFTests,
    &WordTests,
    &PlatformTests,
    NULL
};

void runTest(const char *name)
{
    TestCase *tc = utlookup(allGroups,name);
    if (tc != NULL) {
        tc->fun();
        return;
    }

    DFBufferFormat(utgetoutput(),"Unknown test: %s\n",name);
}
