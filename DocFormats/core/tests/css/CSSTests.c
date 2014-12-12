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
#include "DFHashTable.h"
#include "DFBuffer.h"
#include "CSSSheet.h"
#include "DFCommon.h"
#include <string.h>
#include <stdlib.h>

void test_CSS_setHeadingNumbering(void)
{
    const char *inputCSS = DFHashTableLookup(utgetdata(),"input.css");
    if (inputCSS == NULL) {
        DFBufferFormat(utgetoutput(),"CSS_setHeadingNumbering: input.css not defined");
        return;
    }
    if (utgetargc() < 1) {
        DFBufferFormat(utgetoutput(),"CSS_setHeadingNumbering: expected 1 argument");
        return;
    }

    CSSSheet *styleSheet = CSSSheetNew();
    CSSSheetUpdateFromCSSText(styleSheet,inputCSS);
    int on = !strcasecmp(utgetargv()[0],"true");
    CSSSheetSetHeadingNumbering(styleSheet,on);
    char *cssText = CSSSheetCopyCSSText(styleSheet);
    DFBufferFormat(utgetoutput(),"%s",cssText);
    free(cssText);
    CSSSheetRelease(styleSheet);
}

void test_CSS_parse(void)
{
    const char *inputCSS = DFHashTableLookup(utgetdata(),"input.css");
    if (inputCSS == NULL) {
        DFBufferFormat(utgetoutput(),"input.css not defined");
        return;
    }
    CSSSheet *styleSheet = CSSSheetNew();
    CSSSheetUpdateFromCSSText(styleSheet,inputCSS);
    char *text = CSSSheetCopyText(styleSheet);
    DFBufferFormat(utgetoutput(),"%s",text);
    free(text);
    DFBufferFormat(utgetoutput(),
                   "================================================================================\n");
    char *cssText = CSSSheetCopyCSSText(styleSheet);
    DFBufferFormat(utgetoutput(),"%s",cssText);
    free(cssText);
    CSSSheetRelease(styleSheet);
}

static void test_sample(void)
{
}

TestGroup CSSTests = {
    "core.css", {
        { "sample", PlainTest, test_sample },
        { NULL, PlainTest, NULL }
    }
};
