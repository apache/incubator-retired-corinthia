// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "DFPlatform.h"
#include "DFUnitTest.h"
#include "HTMLPlain.h"
#include "HTMLToLaTeX.h"
#include "DFHTMLNormalization.h"
#include <stdlib.h>

static void test_create(void)
{
    DFError *error = NULL;
    DFStorage *htmlStorage = DFStorageNewMemory(DFFileFormatHTML);
    DFDocument *htmlDoc = TestCaseGetHTML(htmlStorage,&error);
    DFStorageRelease(htmlStorage);
    if (htmlDoc == NULL) {
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    HTML_normalizeDocument(htmlDoc);
    char *latex = HTMLToLaTeX(htmlDoc);
    DFBufferFormat(utgetoutput(),"%s",latex);
    free(latex);
    DFDocumentRelease(htmlDoc);
}

TestGroup LaTeXTests = {
    "latex", {
        { "create", DataTest, test_create },
        { NULL, PlainTest, NULL }
    }
};
