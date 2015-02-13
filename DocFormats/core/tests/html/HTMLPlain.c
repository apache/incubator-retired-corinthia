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
#include "HTMLPlain.h"
#include "TextPackage.h"
#include "DFXML.h"
#include "DFHashTable.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFHTML.h"
#include "DFCommon.h"
#include "DFZipFile.h"
#include "DFUnitTest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void HTML_getImageSourcesRecursive(DFNode *node, DFHashTable *result)
{
    if (node->tag == HTML_IMG) {
        const char *src = DFGetAttribute(node,HTML_SRC);
        if (src != NULL)
            DFHashTableAdd(result,src,"");
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        HTML_getImageSourcesRecursive(child,result);
}

static const char **HTML_getImageSources(DFDocument *doc)
{
    DFHashTable *set = DFHashTableNew(NULL,NULL);
    HTML_getImageSourcesRecursive(doc->docNode,set);
    const char **result = DFHashTableCopyKeys(set);
    DFHashTableRelease(set);
    return result;
}

char *HTML_toPlain(DFDocument *doc, DFStorage *storage, DFError **error)
{
    DFBuffer *output = DFBufferNew();
    char *docStr = DFSerializeXMLString(doc,0,0);
    DFBufferFormat(output,"%s",docStr);
    free(docStr);
    const char **imageSources = HTML_getImageSources(doc);
    DFSortStringsCaseInsensitive(imageSources);
    for (size_t i = 0; imageSources[i]; i++) {
        const char *src = imageSources[i];
        if (DFStringHasPrefix(src,"images/"))
            DFBufferFormat(output,"#item %s\n",src);
        else
            DFBufferFormat(output,"#item images/%s\n",src);
        DFBuffer *imageData = DFBufferReadFromStorage(storage,src,error);
        if (imageData == NULL) {
            DFErrorFormat(error,"%s: %s",src,DFErrorMessage(error));
            return NULL;
        }
        char *imageStr = binaryToString(imageData);
        DFBufferFormat(output,"%s",imageStr);
        free(imageStr);
        DFBufferRelease(imageData);
    }
    free(imageSources);

    char *str = strdup(output->data);
    DFBufferRelease(output);
    return str;
}

static DFDocument *HTML_fromTextPackage(TextPackage *textPackage, DFStorage *htmlStorage, DFError **error)
{
    const char *html = DFHashTableLookup(textPackage->items,"");
    if (html == NULL) {
        DFErrorFormat(error,"No HTML data");
        return NULL;
    }

    DFDocument *doc = DFParseHTMLString(html,0,error);
    if (doc == NULL)
        return NULL;

    for (size_t ki = 0; ki < textPackage->nkeys; ki++) {
        const char *key = textPackage->keys[ki];
        if (strlen(key) == 0)
            continue;

        int ok = 1;

        const char *str = DFHashTableLookup(textPackage->items,key);
        DFBuffer *data = stringToBinary(str);
        if (!DFBufferWriteToStorage(data,htmlStorage,key,error)) {
            DFErrorFormat(error,"%s: %s",key,DFErrorMessage(error));
            DFDocumentRelease(doc);
            ok = 0;
        }

        DFBufferRelease(data);

        if (!ok)
            return NULL;
    }

    return doc;
}

DFDocument *HTML_fromPlain(const char *plain, const char *path, DFStorage *htmlStorage, DFError **error)
{
    TextPackage *textPackage = TextPackageNewWithString(plain,path,error);
    if (textPackage == NULL)
        return NULL;;
    DFDocument *result = HTML_fromTextPackage(textPackage,htmlStorage,error);
    TextPackageRelease(textPackage);
    return result;
}

DFDocument *TestCaseGetHTML(DFStorage *storage, DFError **error)
{
    const char *inputHtml = DFHashTableLookup(utgetdata(),"input.html");
    if (inputHtml == NULL) {
        DFErrorFormat(error,"input.html not defined");
        return NULL;
    }
    return HTML_fromPlain(inputHtml,utgetpath(),storage,error);
}
