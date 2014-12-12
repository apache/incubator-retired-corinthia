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

// BDTTests.c
void test_move(void);
void test_removeChildren(void);

// CSSTests.c
void test_CSS_setHeadingNumbering(void);
void test_CSS_parse(void);

// WordTests.c
void test_Word_testCollapseBookmarks(void);
void test_Word_testExpandBookmarks(void);
void test_Word_testGet(void);
void test_Word_testCreate(void);
void test_Word_testUpdate(void);

// LaTeXTests.c
void test_LaTeX_testCreate(void);

// HTMLTests.c
void test_HTML_testNormalize(void);
void test_HTML_showChanges(void);



typedef void (*TestFunction)(void);

static struct {
    const char *name;
    TestFunction fun;
} testFunctions[] = {
    { "CSS_setHeadingNumbering", test_CSS_setHeadingNumbering },
    { "Word_testCollapseBookmarks", test_Word_testCollapseBookmarks },
    { "Word_testExpandBookmarks", test_Word_testExpandBookmarks },
    { "Word_testGet", test_Word_testGet },
    { "Word_testCreate", test_Word_testCreate },
    { "Word_testUpdate", test_Word_testUpdate },
    { "LaTeX_testCreate", test_LaTeX_testCreate },
    { "HTML_testNormalize", test_HTML_testNormalize },
    { "HTML_showChanges", test_HTML_showChanges },
    { "CSS_test", test_CSS_parse },
    { "move", test_move },
    { "remove", test_removeChildren },
    { NULL, NULL },
};

void runTest(const char *name)
{
    for (int i = 0; testFunctions[i].name != NULL; i++) {
        if (!strcmp(testFunctions[i].name,name)) {
            testFunctions[i].fun();
            return;
        }
    }
    printf("runTest %s\n",name);
    for (int i = 0; i < utgetargc(); i++) {
        printf("    utgetargv()[%d] = %s\n",i,utgetargv()[i]);
    }
    DFBufferFormat(utgetoutput(),"Unknown test: %s\n",name);
}
