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

#include <DocFormats/Operations.h>
#include "DFFilesystem.h"
#include "DFString.h"
#include "DFPackage.h"
#include "WordPackage.h"
#include "DFHTML.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFZipFile.h"
#include <stdlib.h>

static int generateHTML(const char *packageFilename, const char *htmlFilename, DFError **error)
{
    int ok = 0;
    DFPackage *rawPackage = NULL;
    WordPackage *wordPackage = NULL;
    char *htmlPath = DFPathDirName(htmlFilename);
    DFBuffer *warnings = DFBufferNew();
    DFDocument *htmlDoc = NULL;

    rawPackage = DFPackageOpenZip(packageFilename,error);
    if (rawPackage == NULL) {
        DFErrorFormat(error,"%s: %s",packageFilename,DFErrorMessage(error));
        goto end;
    }

    wordPackage = WordPackageOpenFrom(rawPackage,error);
    if (wordPackage == NULL) {
        DFErrorFormat(error,"%s: %s",packageFilename,DFErrorMessage(error));
        goto end;
    }

    htmlDoc = WordPackageGenerateHTML(wordPackage,htmlPath,"word",error,warnings);
    if (htmlDoc == NULL)
        goto end;

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        goto end;
    }

    HTML_safeIndent(htmlDoc->docNode,0);

    if (!DFSerializeXMLFile(htmlDoc,0,0,htmlFilename,error)) {
        DFErrorFormat(error,"%s: %s",htmlFilename,DFErrorMessage(error));
        goto end;
    }

    ok = 1;

end:
    free(htmlPath);
    DFBufferRelease(warnings);
    DFDocumentRelease(htmlDoc);
    DFPackageRelease(rawPackage);
    WordPackageRelease(wordPackage);
    return ok;
}

static int updateFrom(const char *packageFilename, const char *htmlFilename, DFError **error)
{
    int ok = 0;
    DFPackage *rawPackage = NULL;
    WordPackage *wordPackage = NULL;
    DFDocument *htmlDoc = NULL;
    DFBuffer *warnings = DFBufferNew();
    char *htmlPath = DFPathDirName(htmlFilename);

    htmlDoc = DFParseHTMLFile(htmlFilename,0,error);
    if (htmlDoc == NULL) {
        DFErrorFormat(error,"%s: %s",htmlFilename,DFErrorMessage(error));
        goto end;
    }

    const char *idPrefix = "word";

    if (!DFFileExists(packageFilename)) {

        rawPackage = DFPackageCreateZip(packageFilename,error);
        if (rawPackage == NULL) {
            DFErrorFormat(error,"%s: %s",packageFilename,DFErrorMessage(error));
            goto end;
        }

        wordPackage = WordPackageOpenNew(rawPackage,error);
        if (wordPackage == NULL)
            goto end;

        // Change any id attributes starting with "word" or "odf" to a different prefix, so they
        // are not treated as references to nodes in the destination document. This is necessary
        // if the HTML file was previously generated from a word or odf file, and we are creating
        // a new word or odf file from it.
        HTMLBreakBDTRefs(htmlDoc->docNode,idPrefix);
    }
    else {
        rawPackage = DFPackageOpenZip(packageFilename,error);
        if (rawPackage == NULL) {
            DFErrorFormat(error,"%s: %s",packageFilename,DFErrorMessage(error));
            goto end;
        }
        wordPackage = WordPackageOpenFrom(rawPackage,error);
        if (wordPackage == NULL)
            goto end;
    }

    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,htmlPath,idPrefix,error,warnings))
        goto end;

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        goto end;
    }

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;

end:
    DFPackageRelease(rawPackage);
    WordPackageRelease(wordPackage);
    DFDocumentRelease(htmlDoc);
    DFBufferRelease(warnings);
    free(htmlPath);
    return ok;
}

int DFGetFile(const char *concrete, const char *abstract, DFError **error)
{
    int r = 0;
    char *conExt = DFPathExtension(concrete);
    char *absExt = DFPathExtension(abstract);

    if (DFStringEqualsCI(conExt,"docx") && DFStringEqualsCI(absExt,"html")) {
        r = generateHTML(concrete,abstract,error);
    }

//end:
    free(conExt);
    free(absExt);
    return r;
}

int DFPutFile(const char *concrete, const char *abstract, DFError **error)
{
    int r = 0;
    char *conExt = DFPathExtension(concrete);
    char *absExt = DFPathExtension(abstract);

    if (DFStringEqualsCI(conExt,"docx") && DFStringEqualsCI(absExt,"html")) {
        r = updateFrom(concrete,abstract,error);
    }

//end:
    free(conExt);
    free(absExt);
    return r;
}

int DFCreateFile(const char *concrete, const char *abstract, DFError **error)
{
    int r = 0;
    char *conExt = DFPathExtension(concrete);
    char *absExt = DFPathExtension(abstract);

    if (DFStringEqualsCI(conExt,"docx") && DFStringEqualsCI(absExt,"html")) {
        if (DFFileExists(concrete) && !DFDeleteFile(concrete,error))
            goto end;
        r = updateFrom(concrete,abstract,error);
    }

end:
    free(conExt);
    free(absExt);
    return r;
}
