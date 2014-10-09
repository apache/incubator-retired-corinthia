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

#include "Commands.h"
#include "BDTTest.h"
#include "Test.h"
#include "Plain.h"
#include "TextPackage.h"
#include "StringTests.h"
#include "DFChanges.h"
#include "OPC.h"
#include "WordConverter.h"
#include "DFHTML.h"
#include "DFXML.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFHTMLNormalization.h"
#include "CSS.h"
#include "HTMLToLaTeX.h"
#include "DFCommon.h"

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
    char *tempPath = createTempDir(error);
    if (tempPath == NULL)
        return 0;;
    int ok = 0;
    char *wordTempPath = DFAppendPathComponent(tempPath,"word");
    char *plainTempPath = DFAppendPathComponent(tempPath,"plain");
    if (!DFEmptyDirectory(wordTempPath,error))
        goto end;

    char *plain = NULL;
    WordPackage *package = NULL;
    DFStore *store = DFStoreNewFilesystem(tempPath);
    package = WordPackageNew(store);
    DFStoreRelease(store);
    if (!WordPackageOpenFrom(package,filename,error))
        goto end;

    WordPackageRemovePointlessElements(package);
    plain = Word_toPlain(package,NULL,plainTempPath);
    printf("%s",plain);

    ok = 1;

end:
    free(tempPath);
    free(wordTempPath);
    free(plainTempPath);
    free(plain);
    if (package != NULL)
        WordPackageRelease(package);
    DFDeleteFile(tempPath,NULL);
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

static int fromPlain2(const char *tempPath, const char *inStr, const char *inPath,
                      const char *outFilename, DFError **error)
{
    char *outExtension = DFPathExtension(outFilename);
    char *packagePath = DFAppendPathComponent(tempPath,"package");
    char *zipPath = DFAppendPathComponent(tempPath,"zip");
    int isDocx = DFStringEqualsCI(outExtension,"docx");
    int ok = 0;

    if (!isDocx) {
        DFErrorFormat(error,"%s: Unknown extension",outFilename);
        goto end;
    }

    WordPackage *package = Word_fromPlain(inStr,inPath,packagePath,zipPath,error);
    if (package == NULL)
        goto end;

    ok = WordPackageSaveTo(package,outFilename,error);
    WordPackageRelease(package);

    return ok;
end:
    free(packagePath);
    free(zipPath);
    free(outExtension);
    return ok;
}

int fromPlain(const char *inFilename, const char *outFilename, DFError **error)
{
    int fromStdin = !strcmp(inFilename,"-");
    char *inStr = readString(inFilename,error);
    if (inStr == NULL)
        return 0;

    char *tempPath = createTempDir(error);
    if (tempPath == NULL) {
        free(inStr);
        return 0;
    }

    char *inPath = fromStdin ? strdup(".") : DFPathDirName(inFilename);
    int ok = fromPlain2(tempPath,inStr,inPath,outFilename,error);
    DFDeleteFile(tempPath,NULL);
    free(inPath);
    free(tempPath);
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

static int generateHTML(WordPackage *ext, const char *packageFilename, const char *htmlFilename, DFError **error)
{
    if (!WordPackageOpenFrom(ext,packageFilename,error))
        return 0;

    char *htmlPath = DFPathDirName(htmlFilename);
    DFBuffer *warnings = DFBufferNew();
    DFDocument *htmlDoc = WordPackageGenerateHTML(ext,htmlPath,"word",error,warnings);
    free(htmlPath);
    if (htmlDoc == NULL) {
        DFBufferRelease(warnings);
        return 0;
    }

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        DFBufferRelease(warnings);
        DFDocumentRelease(htmlDoc);
        return 0;
    }
    DFBufferRelease(warnings);

    HTML_safeIndent(htmlDoc->docNode,0);

    if (!DFSerializeXMLFile(htmlDoc,0,0,htmlFilename,error)) {
        DFErrorFormat(error,"%s: %s",htmlFilename,DFErrorMessage(error));
        DFDocumentRelease(htmlDoc);
        return 0;
    }

    printf("Created %s\n",htmlFilename);
    DFDocumentRelease(htmlDoc);
    return 1;
}

static int updateFrom(WordPackage *ext, const char *packageFilename,
                       const char *htmlFilename, DFError **error)
{
    DFDocument *htmlDoc = DFParseHTMLFile(htmlFilename,0,error);
    if (htmlDoc == NULL) {
        DFErrorFormat(error,"%s: %s",htmlFilename,DFErrorMessage(error));
        return 0;
    }

    const char *idPrefix = "word";

    if (!DFFileExists(packageFilename)) {
        if (!WordPackageOpenNew(ext,error)) {
            DFDocumentRelease(htmlDoc);
            return 0;
        }
        // Change any id attributes starting with "word" or "odf" to a different prefix, so they
        // are not treated as references to nodes in the destination document. This is necessary
        // if the HTML file was previously generated from a word or odf file, and we are creating
        // a new word or odf file from it.
        HTMLBreakBDTRefs(htmlDoc->docNode,idPrefix);
    }
    else {
        if (!WordPackageOpenFrom(ext,packageFilename,error)) {
            DFDocumentRelease(htmlDoc);
            return 0;
        }
    }

    DFBuffer *warnings = DFBufferNew();
    char *htmlPath = DFPathDirName(htmlFilename);
    if (!WordPackageUpdateFromHTML(ext,htmlDoc,htmlPath,idPrefix,error,warnings)) {
        free(htmlPath);
        DFBufferRelease(warnings);
        DFDocumentRelease(htmlDoc);
        return 0;
    }
    free(htmlPath);
    DFDocumentRelease(htmlDoc);

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        DFBufferRelease(warnings);
        return 0;
    }
    DFBufferRelease(warnings);

    if (!WordPackageSaveTo(ext,packageFilename,error))
        return 0;

    return 1;
}

static int convertHTMLToLaTeX(const char *inFilename, const char *outFilename, DFError **error)
{
    DFDocument *htmlDoc = DFParseHTMLFile(inFilename,0,error);
    if (htmlDoc == NULL) {
        DFErrorFormat(error,"%s: %s",inFilename,DFErrorMessage(error));
        return 0;
    }

    HTML_normalizeDocument(htmlDoc);
    char *latex = HTMLToLaTeX(htmlDoc);


    if (!DFStringWriteToFile(latex,outFilename,error)) {
        DFErrorFormat(error,"%s: %s",inFilename,DFErrorMessage(error));
        free(latex);
        DFDocumentRelease(htmlDoc);
        return 0;
    }

    free(latex);
    DFDocumentRelease(htmlDoc);
    return 1;
}

static int run(const char *inFilename, const char *outFilename, DFError **error, const char *tempPath)
{
    char *inExt = DFPathExtension(inFilename);
    char *outExt = DFPathExtension(outFilename);
    int result = 0;

    if (DFStringEqualsCI(inExt,"docx") && DFStringEqualsCI(outExt,"html")) {
        // Generate new HTML file from .docx
        DFStore *store = DFStoreNewFilesystem(tempPath);
        WordPackage *word = WordPackageNew(store);
        DFStoreRelease(store);
        result = generateHTML(word,inFilename,outFilename,error);
        WordPackageRelease(word);
    }
    else if (DFStringEqualsCI(inExt,"html") && DFStringEqualsCI(outExt,"docx")) {
        // Update existing .docx file from HTML
        DFStore *store = DFStoreNewFilesystem(tempPath);
        WordPackage *word = WordPackageNew(store);
        DFStoreRelease(store);
        result = updateFrom(word,outFilename,inFilename,error);
        WordPackageRelease(word);
    }
    else if (DFStringEqualsCI(inExt,"html") && DFStringEqualsCI(outExt,"tex")) {
        // Create new .tex file from HTML
        result = convertHTMLToLaTeX(inFilename,outFilename,error);
    }
    else {
        DFErrorFormat(error,"Unknown conversion type");
        result = 0;
    }

    free(inExt);
    free(outExt);
    return result;
}

int convertFile(const char *inFilename, const char *outFilename, DFError **error)
{
    char *settemp = getenv("DFUTIL_TEMP");
    char *tempPath;
    if (settemp != NULL) {
        tempPath = strdup(settemp);
    }
    else {
        tempPath = createTempDir(error);
        if (tempPath == NULL)
            return 0;
    }

    int ok = run(inFilename,outFilename,error,tempPath);

    if (settemp == NULL)
        DFDeleteFile(tempPath,NULL);

    free(tempPath);
    return ok;
}

int resaveOPCFile(const char *filename, DFError **error)
{
    DFStore *store = DFStoreNewFilesystem("opc");
    OPCPackage *package = OPCPackageNew(store);
    DFStoreRelease(store);
    if (!OPCPackageOpenFrom(package,filename)) {
        DFErrorFormat(error,"%s",package->errors->data);
        OPCPackageFree(package);
        return 0;
    }
    printf("Package opened successfully\n");
    if (!OPCPackageSaveTo(package,filename)) {
        DFErrorFormat(error,"%s",package->errors->data);
        OPCPackageFree(package);
        return 0;
    }
    printf("Package saved successfully\n");
    OPCPackageFree(package);
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

int simplifyFields(const char *inFilename, const char *outFilename, DFError **error)
{
    char *outExtension = DFPathExtension(outFilename);
    int isDocx = DFStringEqualsCI(outExtension,"docx");
    free(outExtension);

    if (!isDocx) {
        DFErrorFormat(error,"%s: Simplify fields only applies to docx files",inFilename);
        return 0;
    }

    char *tempPath = createTempDir(error);
    if (tempPath == NULL)
        return 0;;

    DFStore *store = DFStoreNewFilesystem(tempPath);
    WordPackage *package = WordPackageNew(store);
    DFStoreRelease(store);
    if (!WordPackageOpenFrom(package,inFilename,error)) {
        DFErrorFormat(error,"%s: %s",inFilename,DFErrorMessage(error));
        WordPackageRelease(package);
        DFDeleteFile(tempPath,NULL);
        free(tempPath);
        return 0;
    }

    WordPackageSimplifyFields(package);

    if (!WordPackageSaveTo(package,outFilename,error)) {
        DFErrorFormat(error,"%s: %s",outFilename,DFErrorMessage(error));
        WordPackageRelease(package);
        DFDeleteFile(tempPath,NULL);
        free(tempPath);
        return 0;
    }

    WordPackageRelease(package);
    DFDeleteFile(tempPath,NULL);
    free(tempPath);
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

int runTests(int argc, const char **argv, int diff, DFError **error)
{
    DFArray *tests = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (int i = 0; i < argc; i++) {
        const char *path = argv[i];
        if (!DFFileExists(path)) {
            DFErrorFormat(error,"%s: No such file or directory",path);
            DFArrayRelease(tests);
            return 0;
        }
        TestGetFilenamesRecursive(path,tests);
    }

    TestHarness harness;
    bzero(&harness,sizeof(TestHarness));
    harness.showResults = (DFArrayCount(tests) == 1);
    harness.showDiffs = diff;

    if (DFArrayCount(tests) == 1) {
        TestRun(&harness,(const char *)DFArrayItemAt(tests,0));
    }
    else {
        for (size_t i = 0; i < DFArrayCount(tests); i++) {
            const char *test = DFArrayItemAt(tests,i);
            TestRun(&harness,test);
        }
        printf("Passed: %d\n",harness.passed);
        printf("Failed: %d\n",harness.failed);
    }
    DFArrayRelease(tests);
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
    printf("parts.count = %lu\n",DFArrayCount(parts));
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

char *createTempDir(DFError **error)
{
#ifdef WIN32
    // Windows lacks mkdtemp. For the purposes of a single-threaded app it's ok just to use the same name
    // each time we do a conversion, as long as we ensure that the directory is cleared first.
    const char *name = "dfutil.temp";
    if (DFFileExists(name) && !DFDeleteFile(name,error))
        return NULL;
    if (!DFCreateDirectory(name,1,error))
        return NULL;
    return strdup(name);
#else
    char *ctemplate = strdup("dfutil.XXXXXX");
    char *r = mkdtemp(ctemplate);
    if (r == NULL) {
        DFErrorFormat(error,"mkdtemp: %s",strerror(errno));
        free(ctemplate);
        return NULL;
    }
    return ctemplate;
#endif
}

char *binaryToString(DFBuffer *input)
{
    const char *hexchars = "0123456789ABCDEF";
    DFBuffer *charBuf = DFBufferNew();
    for (size_t pos = 0; pos < input->len; pos++) {
        if ((pos > 0) && (pos % 40 == 0))
            DFBufferAppendChar(charBuf,'\n');
        unsigned char hi = ((unsigned char *)input->data)[pos] >> 4;
        unsigned char lo = ((unsigned char *)input->data)[pos] & 0x0F;
        DFBufferAppendChar(charBuf,hexchars[hi]);
        DFBufferAppendChar(charBuf,hexchars[lo]);
    }
    if ((input->len % 40) != 0)
        DFBufferAppendChar(charBuf,'\n');
    char *result = strdup(charBuf->data);

    DFBufferRelease(charBuf);
    return result;
}

DFBuffer *stringToBinary(const char *str)
{
    size_t length = strlen(str);
    DFBuffer *outbuf = DFBufferNew();

    int wantHi = 1;
    unsigned char hi = 0;

    for (size_t inpos = 0; inpos < length; inpos++) {
        char c = str[inpos];
        unsigned char nibble = 0;

        if ((c >= '0') && (c <= '9'))
            nibble = c - '0';
        else if ((c >= 'a') && (c <= 'f'))
            nibble = 10 + (c - 'a');
        else if ((c >= 'A') && (c <= 'F'))
            nibble = 10 + (c - 'A');
        else
            continue;

        if (wantHi)
            hi = nibble << 4;
        else
            DFBufferAppendChar(outbuf,hi | nibble);
        wantHi = !wantHi;
    }

    return outbuf;
}
