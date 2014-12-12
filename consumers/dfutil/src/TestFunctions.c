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
#include "BDTTest.h"
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

typedef void (*TestFunction)(void);

static void move(void)
{
    if (utgetargc() < 3) {
        DFBufferFormat(utgetoutput(),"move: insufficient arguments");
        return;
    }

    int count = atoi(utgetargv()[0]);
    int from = atoi(utgetargv()[1]);
    int to = atoi(utgetargv()[2]);

    DFBuffer *output = DFBufferNew();
    BDT_testMove(count,from,to,output);
    DFBufferFormat(utgetoutput(),"%s",output->data);
    DFBufferRelease(output);
}

static void removeChildren(void)
{
    int *indices = (int *)malloc(utgetargc()*sizeof(int));

    for (int i = 0; i < utgetargc(); i++) {
        int index = atoi(utgetargv()[i]);
        indices[i] = index;
    }

    DFBuffer *output = DFBufferNew();
    BDT_testRemove(indices,utgetargc(),output);
    DFBufferFormat(utgetoutput(),"%s",output->data);
    DFBufferRelease(output);

    free(indices);
}

static void CSS_setHeadingNumbering(void)
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

static void Word_testCollapseBookmarks(void)
{
    DFError *error = NULL;
    DFStorage *storage = TestCaseOpenPackage(&error);
    if (storage == NULL) {
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    if (!WordCollapseBookmarks(storage,&error)) {
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        DFStorageRelease(storage);
        return;
    }

    DFHashTable *parts = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(parts,"document","");

    // Output the docx file
    char *plain = Word_toPlain(storage,parts);
    DFBufferFormat(utgetoutput(),"%s",plain);
    free(plain);
    DFHashTableRelease(parts);
    DFStorageRelease(storage);
}


static void Word_testExpandBookmarks(void)
{
    DFError *error = NULL;
    DFStorage *storage = TestCaseOpenPackage(&error);
    if (storage == NULL) {
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return;
    }

    if (!WordExpandBookmarks(storage,&error)) {
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        DFStorageRelease(storage);
        return;
    }

    DFHashTable *parts = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(parts,"document","");

    // Output the docx file
    char *plain = Word_toPlain(storage,parts);
    DFBufferFormat(utgetoutput(),"%s",plain);
    free(plain);
    DFHashTableRelease(parts);
    DFStorageRelease(storage);
}

static void Word_testGet(void)
{
    DFError *error = NULL;
    DFStorage *abstractStorage = NULL;
    DFStorage *concreteStorage = NULL;
    DFAbstractDocument *abstractDoc = NULL;
    DFConcreteDocument *concreteDoc = NULL;
    char *htmlPlain = NULL;

    concreteStorage = TestCaseOpenPackage(&error);
    if (concreteStorage == NULL)
        goto end;

    concreteDoc = DFConcreteDocumentNew(concreteStorage);
    abstractStorage = DFStorageNewMemory(DFFileFormatHTML);
    abstractDoc = DFAbstractDocumentNew(abstractStorage);

    if (!DFGet(concreteDoc,abstractDoc,&error))
        goto end;;

    DFDocument *htmlDoc = DFAbstractDocumentGetHTML(abstractDoc);
    if (htmlDoc == NULL) {
        DFErrorFormat(&error,"Abstract document has no HTML");
        goto end;
    }

    htmlPlain = HTML_toPlain(htmlDoc,abstractStorage,&error);

end:
    if (htmlPlain != NULL)
        DFBufferFormat(utgetoutput(),"%s",htmlPlain);
    else
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));

    DFErrorRelease(error);
    DFStorageRelease(abstractStorage);
    DFStorageRelease(concreteStorage);
    DFAbstractDocumentRelease(abstractDoc);
    DFConcreteDocumentRelease(concreteDoc);
    free(htmlPlain);
}

static DFHashTable *getFlags(void)
{
    if (utgetargc() == 0)
        return NULL;;
    DFHashTable *set = DFHashTableNew((DFCopyFunction)strdup,free);
    for (int i = 0; i < utgetargc(); i++) {
        const char *colon = strchr(utgetargv()[i],':');
        if (colon == NULL)
            continue;
        size_t colonPos = colon - utgetargv()[i];
        char *rawName = DFSubstring(utgetargv()[i],0,colonPos);
        char *rawValue = DFSubstring(utgetargv()[i],colonPos+1,strlen(utgetargv()[i]));
        char *name = DFStringTrimWhitespace(rawName);
        char *value = DFStringTrimWhitespace(rawValue);
        if (!strcasecmp(value,"true"))
            DFHashTableAdd(set,name,name);

        free(rawName);
        free(rawValue);
        free(name);
        free(value);
    }
    return set;
}

static void Word_testCreate(void)
{
    DFError *error = NULL;
    DFDocument *htmlDoc = NULL;
    DFStorage *abstractStorage = DFStorageNewMemory(DFFileFormatHTML);
    DFStorage *concreteStorage = DFStorageNewMemory(DFFileFormatDocx);
    DFAbstractDocument *abstractDoc = DFAbstractDocumentNew(abstractStorage);
    DFConcreteDocument *concreteDoc = DFConcreteDocumentNew(concreteStorage);
    DFHashTable *parts = NULL;
    char *wordPlain = NULL;

    // Read input.html
    htmlDoc = TestCaseGetHTML(abstractStorage,&error);
    if (htmlDoc == NULL)
        goto end;

    DFAbstractDocumentSetHTML(abstractDoc,htmlDoc);

    if (!DFCreate(concreteDoc,abstractDoc,&error))
        goto end;

    parts = getFlags();
    wordPlain = Word_toPlain(concreteStorage,parts);

end:
    if (wordPlain != NULL)
        DFBufferFormat(utgetoutput(),"%s",wordPlain);
    else
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));

    DFErrorRelease(error);
    DFDocumentRelease(htmlDoc);
    DFStorageRelease(abstractStorage);
    DFStorageRelease(concreteStorage);
    DFAbstractDocumentRelease(abstractDoc);
    DFConcreteDocumentRelease(concreteDoc);
    DFHashTableRelease(parts);
    free(wordPlain);
}

static void Word_testUpdate(void)
{
    DFError *error = NULL;
    DFDocument *htmlDoc = NULL;
    DFStorage *abstractStorage = NULL;
    DFStorage *concreteStorage = NULL;
    DFAbstractDocument *abstractDoc = NULL;
    DFConcreteDocument *concreteDoc = NULL;
    DFHashTable *parts = NULL;
    char *wordPlain = NULL;

    concreteStorage = TestCaseOpenPackage(&error);
    if (concreteStorage == NULL)
        goto end;

    abstractStorage = DFStorageNewMemory(DFFileFormatHTML);
    abstractDoc = DFAbstractDocumentNew(abstractStorage);
    concreteDoc = DFConcreteDocumentNew(concreteStorage);

    // Read input.html
    htmlDoc = TestCaseGetHTML(abstractStorage,&error);
    if (htmlDoc == NULL)
        goto end;

    DFAbstractDocumentSetHTML(abstractDoc,htmlDoc);

    // Update the docx file based on the contents of the HTML file
    if (!DFPut(concreteDoc,abstractDoc,&error))
        goto end;

    // Output the updated docx file
    parts = getFlags();
    wordPlain = Word_toPlain(concreteStorage,parts);

end:
    if (wordPlain != NULL)
        DFBufferFormat(utgetoutput(),"%s",wordPlain);
    else
        DFBufferFormat(utgetoutput(),"%s\n",DFErrorMessage(&error));

    DFErrorRelease(error);
    DFDocumentRelease(htmlDoc);
    DFStorageRelease(abstractStorage);
    DFStorageRelease(concreteStorage);
    DFAbstractDocumentRelease(abstractDoc);
    DFConcreteDocumentRelease(concreteDoc);
    DFHashTableRelease(parts);
    free(wordPlain);
}

static void LaTeX_testCreate(void)
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

static void HTML_testNormalize(void)
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

static void HTML_showChanges(void)
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

static void CSS_test(void)
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

static struct {
    const char *name;
    TestFunction fun;
} testFunctions[] = {
    { "CSS_setHeadingNumbering", CSS_setHeadingNumbering },
    { "Word_testCollapseBookmarks", Word_testCollapseBookmarks },
    { "Word_testExpandBookmarks", Word_testExpandBookmarks },
    { "Word_testGet", Word_testGet },
    { "Word_testCreate", Word_testCreate },
    { "Word_testUpdate", Word_testUpdate },
    { "LaTeX_testCreate", LaTeX_testCreate },
    { "HTML_testNormalize", HTML_testNormalize },
    { "HTML_showChanges", HTML_showChanges },
    { "CSS_test", CSS_test },
    { "move", move },
    { "remove", removeChildren },
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
