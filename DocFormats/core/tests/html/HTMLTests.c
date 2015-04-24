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
#include "DFHTML.h"
#include "DFHTMLNormalization.h"
#include "DFXML.h"
#include "DFDOM.h"
#include "DFChanges.h"
#include <stdlib.h>

static void test_normalize(void)
{
    const char *inputHtml = DFHashTableLookup(utgetdata(),"input.html");
    if (inputHtml == NULL) {
        DFBufferFormat(utgetoutput(),"input.html not defined");
        return;
    }
    DFError *error = NULL;
    DFDocument *doc = DFParseHTMLString(inputHtml,0,&error);
    if (doc == NULL) {
        DFBufferFormat(utgetoutput(),"%s",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }
    HTML_normalizeDocument(doc);
    HTML_safeIndent(doc->docNode,0);
    char *docStr = DFSerializeXMLString(doc,0,0);
    DFBufferFormat(utgetoutput(),"%s",docStr);
    free(docStr);
    DFDocumentRelease(doc);
}

static void test_showChanges(void)
{
    const char *input1 = DFHashTableLookup(utgetdata(),"input1.html");
    if (input1 == NULL) {
        DFBufferFormat(utgetoutput(),"input.html not defined");
        return;
    }

    const char *input2 = DFHashTableLookup(utgetdata(),"input2.html");
    if (input2 == NULL) {
        DFBufferFormat(utgetoutput(),"input.html not defined");
        return;
    }


    DFError *error = NULL;
    DFDocument *doc1 = DFParseHTMLString(input1,0,&error);
    if (doc1 == NULL) {
        DFBufferFormat(utgetoutput(),"%s",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    DFDocument *doc2 = DFParseHTMLString(input2,0,&error);
    if (doc2 == NULL) {
        DFBufferFormat(utgetoutput(),"%s",DFErrorMessage(&error));
        DFErrorRelease(error);
        DFDocumentRelease(doc1);
    }

    DFComputeChanges(doc1->root,doc2->root,HTML_ID);

    char *changesStr = DFChangesToString(doc1->root);
    DFBufferFormat(utgetoutput(),"%s",changesStr);
    free(changesStr);
    DFDocumentRelease(doc1);
    DFDocumentRelease(doc2);
}

TestGroup HTMLTests = {
    "core.html", {
        { "normalize", DataTest, test_normalize },
        { "showChanges", DataTest, test_showChanges },
        { NULL, PlainTest, NULL }
    }
};
