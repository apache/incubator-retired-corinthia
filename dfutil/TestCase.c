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

#include "TestCase.h"
#include "TestFunctions.h"
#include "Plain.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            TestCase                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

TestCase *TestCaseNew(const char *path, DFHashTable *input)
{
    TestCase *tc = (TestCase *)calloc(1,sizeof(TestCase));
    tc->path = strdup(path);
    tc->input = DFHashTableRetain(input);
    tc->output = DFBufferNew();
    return tc;
}

void TestCaseFree(TestCase *tc)
{
    free(tc->path);
    DFHashTableRelease(tc->input);
    DFBufferRelease(tc->output);
    free(tc->tempPath);
    free(tc->concretePath);
    free(tc->abstractPath);
    free(tc);
}

int TestCaseGetWordPackage(TestCase *tc, WordPackage *package)
{
    DFError *error = NULL;
    const char *inputDocx = DFHashTableLookup(tc->input,"input.docx");
    if (inputDocx == NULL) {
        DFBufferFormat(tc->output,"input.docx not defined\n");
        return 0;
    }
    if (!WordPackageOpenNew(package,&error)) {
        DFBufferFormat(tc->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return 0;
    }
    if (!Word_fromPlain(package,inputDocx,tc->path,&error)) {
        DFBufferFormat(tc->output,"%s\n",DFErrorMessage(&error));
        return 0;
    }
    if (package->document == NULL) {
        DFBufferFormat(tc->output,"document.xml not found\n");
        return 0;
    }
    return 1;
}

DFDocument *TestCaseGetHTML(TestCase *tc)
{
    const char *inputHtml = DFHashTableLookup(tc->input,"input.html");
    if (inputHtml == NULL) {
        DFBufferFormat(tc->output,"input.html not defined\n");
        return NULL;
    }
    DFError *error = NULL;
    DFDocument *htmlDoc = HTML_fromPlain(inputHtml,tc->path,tc->abstractPath,&error);
    if (htmlDoc == NULL) {
        DFBufferFormat(tc->output,"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
    }
    return htmlDoc;
}
