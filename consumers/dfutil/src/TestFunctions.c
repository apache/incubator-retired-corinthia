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

static struct {
    const char *oldName;
    const char *newName;
} substitutions[] = {
    { "CSS_setHeadingNumbering", "core.css.setHeadingNumbering" },
    { "Word_testCollapseBookmarks", "ooxml.word.collapseBookmarks" },
    { "Word_testExpandBookmarks", "ooxml.word.expandBookmarks" },
    { "Word_testGet", "ooxml.word.get" },
    { "Word_testCreate", "ooxml.word.create" },
    { "Word_testUpdate", "ooxml.word.put" },
    { "LaTeX_testCreate", "latex.create" },
    { "HTML_testNormalize", "core.html.normalize" },
    { "HTML_showChanges", "core.html.showChanges" },
    { "CSS_test", "core.css.parse" },
    { "move", "core.bdt.move" },
    { "remove", "core.bdt.removeChildren" },
    { NULL, NULL },
};

void runTest(const char *name)
{
    const char *actualName = name;
    for (int i = 0; substitutions[i].oldName != NULL; i++) {
        if (!strcmp(name,substitutions[i].oldName)) {
            actualName = substitutions[i].newName;
        }
    }
    TestCase *tc = utlookup(allGroups,actualName);
    if (tc != NULL) {
        tc->fun();
        return;
    }

    printf("runTest %s\n",name);
    for (int i = 0; i < utgetargc(); i++) {
        printf("    utgetargv()[%d] = %s\n",i,utgetargv()[i]);
    }
    DFBufferFormat(utgetoutput(),"Unknown test: %s\n",name);
}
