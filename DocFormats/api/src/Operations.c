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

struct DFConcreteDocument {
    size_t retainCount;
    DFPackage *package;
};

struct DFAbstractDocument {
    size_t retainCount;
    DFPackage *package;
};

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

    DFPackage *abstractPackage = DFPackageNewFilesystem(htmlPath,DFFileFormatHTML);
    htmlDoc = WordPackageGenerateHTML(wordPackage,abstractPackage,"word",error,warnings);
    DFPackageRelease(abstractPackage);
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
    DFPackage *abstractPackage = DFPackageNewFilesystem(htmlPath,DFFileFormatHTML);

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

    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,abstractPackage,idPrefix,error,warnings))
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
    DFPackageRelease(abstractPackage);
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

DFConcreteDocument *DFConcreteDocumentNew(DFPackage *package)
{
    DFConcreteDocument *concrete = (DFConcreteDocument *)calloc(1,sizeof(DFConcreteDocument));
    concrete->retainCount = 1;
    concrete->package = DFPackageRetain(package);
    return concrete;
}

DFConcreteDocument *DFConcreteDocumentCreateFile(const char *filename, DFError **error)
{
    DFFileFormat format = DFFileFormatFromFilename(filename);
    switch (format) {
        case DFFileFormatDocx:
        case DFFileFormatXlsx:
        case DFFileFormatPptx:
        case DFFileFormatOdt:
        case DFFileFormatOds:
        case DFFileFormatOdp: {
            DFPackage *package = DFPackageCreateZip(filename,error);
            if (package == NULL)
                return NULL;;
            DFConcreteDocument *concrete = DFConcreteDocumentNew(package);
            DFPackageRelease(package);
            return concrete;
        }
        default:
            DFErrorFormat(error,"Unsupported format for DFConcreteDocumentCreateFile");
            return NULL;
    }
}

DFConcreteDocument *DFConcreteDocumentOpenFile(const char *filename, DFError **error)
{
    DFFileFormat format = DFFileFormatFromFilename(filename);
    switch (format) {
        case DFFileFormatDocx:
        case DFFileFormatXlsx:
        case DFFileFormatPptx:
        case DFFileFormatOdt:
        case DFFileFormatOds:
        case DFFileFormatOdp: {
            DFPackage *package = DFPackageOpenZip(filename,error);
            if (package == NULL)
                return NULL;;
            DFConcreteDocument *concrete = DFConcreteDocumentNew(package);
            DFPackageRelease(package);
            return concrete;
        }
        default:
            DFErrorFormat(error,"Unsupported format for DFConcreteDocumentCreateFile");
            return NULL;
    }
}

DFConcreteDocument *DFConcreteDocumentRetain(DFConcreteDocument *concrete)
{
    if (concrete != NULL)
        concrete->retainCount++;
    return concrete;
}

void DFConcreteDocumentRelease(DFConcreteDocument *concrete)
{
    if ((concrete == NULL) || (--concrete->retainCount > 0))
        return;

    DFPackageRelease(concrete->package);
    free(concrete);
}

DFAbstractDocument *DFAbstractDocumentNew(DFPackage *package)
{
    DFAbstractDocument *abstract = (DFAbstractDocument *)calloc(1,sizeof(DFAbstractDocument));
    abstract->retainCount = 1;
    abstract->package = DFPackageRetain(package);
    return abstract;
}

DFAbstractDocument *DFAbstractDocumentRetain(DFAbstractDocument *abstract)
{
    if (abstract != NULL)
        abstract->retainCount++;
    return abstract;
}

void DFAbstractDocumentRelease(DFAbstractDocument *abstract)
{
    if ((abstract == NULL) || (--abstract->retainCount > 0))
        return;

    DFPackageRelease(abstract->package);
    free(abstract);
}

int DFGet(DFConcreteDocument *concrete, DFAbstractDocument *abstract, DFError **error)
{
    DFErrorFormat(error,"DFGet not yet implemented");
    return 0;
}

int DFPut(DFConcreteDocument *concrete, DFAbstractDocument *abstract, DFError **error)
{
    DFErrorFormat(error,"DFPut not yet implemented");
    return 0;
}

int DFCreate(DFConcreteDocument *concrete, DFAbstractDocument *abstract, DFError **error)
{
    DFErrorFormat(error,"DFCreate not yet implemented");
    return 0;
}
