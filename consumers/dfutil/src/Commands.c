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
#include "Commands.h"
#include "BDTTests.h"
#include "WordPlain.h"
#include "HTMLPlain.h"
#include "TextPackage.h"
#include "StringTests.h"
#include "DFChanges.h"
#include "WordConverter.h"
#include "DFHTML.h"
#include "DFXML.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFHTMLNormalization.h"
#include "CSS.h"
#include "HTMLToLaTeX.h"
#include "DFCommon.h"
#include "DFZipFile.h"
#include <DocFormats/DocFormats.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DFBuffer *readData(const char *filename, DFError **error)
{
    if ((filename == NULL) || !strcmp(filename,"-"))
        filename = "/dev/stdin";;
    DFBuffer *buffer = DFBufferReadFromFile(filename,error);
    if (buffer == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return NULL;
    }
    return buffer;
}

static char *readString(const char *filename, DFError **error)
{
    DFBuffer *buffer = readData(filename,error);
    if (buffer == NULL)
        return NULL;
    char *result = strdup(buffer->data);
    DFBufferRelease(buffer);
    return result;
}

static int writeData(DFBuffer *buf, const char *filename, DFError **error)
{
    if ((filename == NULL) || !strcmp(filename,"-")) {
        fwrite(buf->data,buf->len,1,stdout);
        return 1;
    }
    else if (!DFBufferWriteToFile(buf,filename,error)) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return 0;
    }
    else {
        return 1;
    }
}

static int writeString(const char *str, const char *filename, DFError **error)
{
    DFBuffer *buf = DFBufferNew();
    DFBufferAppendString(buf,str);
    int ok = writeData(buf,filename,error);
    DFBufferRelease(buf);
    return ok;
}

static int prettyPrintXMLFile(const char *filename, int html, DFError **error)
{
    DFError *err = NULL;
    DFDocument *doc;
    if (html)
        doc = DFParseHTMLFile(filename,0,&err);
    else
        doc = DFParseXMLFile(filename,&err);
    if (doc == NULL)
        return 0;

    char *str = DFSerializeXMLString(doc,0,1);
    printf("%s",str);
    free(str);
    DFDocumentRelease(doc);
    return 1;
}

static int prettyPrintWordFile(const char *filename, DFError **error)
{
    int ok = 0;
    char *plain = NULL;
    DFStorage *storage = NULL;

    storage = DFStorageOpenZip(filename,error);
    if (storage == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        goto end;
    }

    plain = Word_toPlain(storage,NULL);
    printf("%s",plain);

    ok = 1;

end:
    free(plain);
    DFStorageRelease(storage);
    return ok;
}

int prettyPrintFile(const char *filename, DFError **error)
{
    int ok;
    char *extension = DFPathExtension(filename);
    if (DFStringEqualsCI(extension,"xml"))
        ok = prettyPrintXMLFile(filename,0,error);
    else if (DFStringEqualsCI(extension,"html") || DFStringEqualsCI(extension,"htm"))
        ok = prettyPrintXMLFile(filename,1,error);
    else if (DFStringEqualsCI(extension,"docx"))
        ok = prettyPrintWordFile(filename,error);
    else {
        DFErrorFormat(error,"Unknown file type");
        ok = 0;
    }
    free(extension);
    return ok;
}

static int fromPlain2(const char *inStr, const char *inPath, const char *outFilename, DFError **error)
{
    char *outExtension = DFPathExtension(outFilename);
    int isDocx = DFStringEqualsCI(outExtension,"docx");
    int ok = 0;

    if (!isDocx) {
        DFErrorFormat(error,"%s: Unknown extension",outFilename);
        goto end;
    }

    DFStorage *storage = NULL;

    storage = Word_fromPlain(inStr,inPath,error);
    if (storage == NULL)
        goto end;

    ok = DFZip(outFilename,storage,error);
    DFStorageRelease(storage);

    return ok;
end:
    free(outExtension);
    return ok;
}

int fromPlain(const char *inFilename, const char *outFilename, DFError **error)
{
    int fromStdin = !strcmp(inFilename,"-");
    char *inStr = readString(inFilename,error);
    if (inStr == NULL)
        return 0;

    char *inPath = fromStdin ? strdup(".") : DFPathDirName(inFilename);
    int ok = fromPlain2(inStr,inPath,outFilename,error);
    free(inPath);
    free(inStr);
    return ok;
}

int normalizeFile(const char *filename, DFError **error)
{
    DFDocument *doc = DFParseHTMLFile(filename,0,error);
    if (doc == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return 0;
    }

    HTML_normalizeDocument(doc);
    HTML_safeIndent(doc->docNode,0);
    char *str = DFSerializeXMLString(doc,0,0);
    printf("%s",str);
    free(str);
    DFDocumentRelease(doc);
    return 1;
}

int testCSS(const char *filename, DFError **error)
{
    char *input = DFStringReadFromFile(filename,error);
    if (input == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return 0;
    }

    CSSSheet *styleSheet = CSSSheetNew();
    CSSSheetUpdateFromCSSText(styleSheet,input);
    char *text = CSSSheetCopyText(styleSheet);
    printf("%s",text);
    free(text);
    printf("================================================================================\n");
    char *cssText = CSSSheetCopyCSSText(styleSheet);
    printf("%s",cssText);
    free(cssText);
    CSSSheetRelease(styleSheet);
    free(input);

    return 1;
}

int parseHTMLFile(const char *filename, DFError **error)
{
    DFDocument *doc = DFParseHTMLFile(filename,0,error);
    if (doc == NULL)
        return 0;
    char *result = DFSerializeXMLString(doc,0,0);
    printf("%s",result);
    free(result);
    DFDocumentRelease(doc);
    return 1;
}

static int textPackageListRecursive(const char *input, const char *filePath, DFError **error, int indent)
{
    TextPackage *package = TextPackageNewWithString(input,filePath,error);
    if (package == NULL)
        return 0;
    if (package->nkeys == 1) {
        TextPackageRelease(package);
        return 1;
    }
    for (size_t ki = 0; ki < package->nkeys; ki++) {
        const char *key = package->keys[ki];
        if (strlen(key) == 0)
            continue;
        for (int i = 0; i < indent; i++)
            printf("    ");
        printf("%s\n",key);
        const char *value = DFHashTableLookup(package->items,key);
        if (!textPackageListRecursive(value,filePath,error,indent+1)) {
            TextPackageRelease(package);
            return 0;
        }
    }
    TextPackageRelease(package);
    return 1;
}

int textPackageList(const char *filename, DFError **error)
{
    char *filePath = DFPathDirName(filename);
    char *value = DFStringReadFromFile(filename,error);
    int result = 0;
    if (value == NULL)
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
    else if (!textPackageListRecursive(value,filePath,error,0))
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
    else
        result = 1;

    free(filePath);
    free(value);
    return result;
}

int textPackageGet(const char *filename, const char *itemPath, DFError **error)
{
    char *value = DFStringReadFromFile(filename,error);
    if (value == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return 0;
    }

    const char **components = DFStringSplit(itemPath,"/",0);
    for (size_t i = 0; components[i]; i++) {
        const char *name = components[i];
        char *filePath = DFPathDirName(filename);
        TextPackage *package = TextPackageNewWithString(value,filePath,error);
        free(filePath);
        if (package == NULL) {
            free(value);
            free(components);
            return 0;
        }
        free(value);
        value = strdup(DFHashTableLookup(package->items,name));
        if (value == NULL) {
            DFErrorFormat(error,"%s: Item %s not found",filename,itemPath);
            TextPackageRelease(package);
            free(value);
            free(components);
            return 0;
        }
        TextPackageRelease(package);
    }
    free(components);

    printf("%s",value);
    free(value);
    return 1;
}

int diffFiles(const char *filename1, const char *filename2, DFError **error)
{
    DFDocument *doc1 = DFParseHTMLFile(filename1,0,error);
    if (doc1 == NULL) {
        DFErrorFormat(error,"%s: %s",filename1,DFErrorMessage(error));
        return 0;
    }

    DFDocument *doc2 = DFParseHTMLFile(filename1,0,error);
    if (doc2 == NULL) {
        DFErrorFormat(error,"%s: %s",filename2,DFErrorMessage(error));
        DFDocumentRelease(doc1);
        return 0;
    }

    DFComputeChanges(doc1->root,doc2->root,HTML_ID);
    char *changesStr = DFChangesToString(doc1->root);
    printf("%s",changesStr);
    free(changesStr);

    DFDocumentRelease(doc1);
    DFDocumentRelease(doc2);
    return 1;
}

void parseContent(const char *content)
{
    DFArray *parts = CSSParseContent(content);
    printf("parts.count = %zu\n",DFArrayCount(parts));
    for (size_t i = 0; i < DFArrayCount(parts); i++) {
        ContentPart *part = DFArrayItemAt(parts,i);
        char *quotedValue = DFQuote(part->value);
        printf("%s %s\n",ContentPartTypeString(part->type),quotedValue);
        free(quotedValue);
    }
    DFArrayRelease(parts);
}

int btosFile(const char *inFilename, const char *outFilename, DFError **error)
{
    DFBuffer *data = readData(inFilename,error);
    if (data == NULL)
        return 0;
    char *str = binaryToString(data);
    int ok = writeString(str,outFilename,error);
    free(str);
    DFBufferRelease(data);
    return ok;
}

int stobFile(const char *inFilename, const char *outFilename, DFError **error)
{
    char *str = readString(inFilename,error);
    if (str == NULL)
        return 0;;
    DFBuffer *bin = stringToBinary(str);
    int ok = writeData(bin,outFilename,error);
    DFBufferRelease(bin);
    free(str);
    return ok;
}

int escapeCSSIdent(const char *filename, DFError **error)
{
    char *input = readString(filename,error);
    if (input == NULL)
        return 0;
    char *unescaped = DFStringTrimWhitespace(input);
    char *escaped = CSSEscapeIdent(unescaped);
    printf("%s\n",escaped);
    free(escaped);
    free(unescaped);
    free(input);
    return 1;
}

int unescapeCSSIdent(const char *filename, DFError **error)
{
    char *input = readString(filename,error);
    if (input == NULL)
        return 0;
    char *escaped = DFStringTrimWhitespace(input);
    char *unescaped = CSSUnescapeIdent(escaped);
    printf("%s\n",unescaped);
    free(unescaped);
    free(escaped);
    free(input);
    return 1;
}

