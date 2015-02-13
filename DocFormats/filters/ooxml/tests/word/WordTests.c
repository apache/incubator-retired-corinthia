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
#include "DFCommon.h"
#include "WordPlain.h"
#include "Word.h"
#include "HTMLPlain.h"
#include "DFString.h"
#include <DocFormats/Operations.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

static void test_collapseBookmarks(void)
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

static void test_expandBookmarks(void)
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

static void test_get(void)
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

static void test_create(void)
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

static void test_put(void)
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

TestGroup WordTests = {
    "ooxml.word", {
        { "collapseBookmarks", DataTest, test_collapseBookmarks },
        { "expandBookmarks", DataTest, test_expandBookmarks },
        { "get", DataTest, test_get },
        { "create", DataTest, test_create },
        { "put", DataTest, test_put },
        { NULL, PlainTest, NULL }
    }
};
