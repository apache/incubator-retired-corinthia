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
#include "Word.h"
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
    DFDocument *htmlDoc;
};

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
    DFDocumentRelease(abstract->htmlDoc);
    free(abstract);
}

int DFGet(DFConcreteDocument *concrete, DFAbstractDocument *abstract, DFError **error)
{
    if (DFPackageFormat(abstract->package) != DFFileFormatHTML) {
        DFErrorFormat(error,"Abstract document must be in HTML format");
        return 0;
    }

    DFDocument *htmlDoc = NULL;
    switch (DFPackageFormat(concrete->package)) {
        case DFFileFormatDocx:
            htmlDoc = WordGet(concrete->package,abstract->package,error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }

    if (htmlDoc == NULL)
        return 0;

    DFDocumentRelease(abstract->htmlDoc);
    abstract->htmlDoc = htmlDoc;
    return 1;
}

int DFPut(DFConcreteDocument *concreteDoc, DFAbstractDocument *abstractDoc, DFError **error)
{
    if (DFPackageFormat(abstractDoc->package) != DFFileFormatHTML) {
        DFErrorFormat(error,"Abstract document must be in HTML format");
        return 0;
    }

    int ok = 0;
    switch (DFPackageFormat(concreteDoc->package)) {
        case DFFileFormatDocx:
            ok = WordPut(concreteDoc->package,abstractDoc->package,abstractDoc->htmlDoc,error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }
    return ok;
}

int DFCreate(DFConcreteDocument *concreteDoc, DFAbstractDocument *abstractDoc, DFError **error)
{
    if (DFPackageFormat(abstractDoc->package) != DFFileFormatHTML) {
        DFErrorFormat(error,"Abstract document must be in HTML format");
        return 0;
    }

    int ok = 0;
    switch (DFPackageFormat(concreteDoc->package)) {
        case DFFileFormatDocx:
            ok = WordCreate(concreteDoc->package,abstractDoc->package,abstractDoc->htmlDoc,error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }
    return ok;
}

int DFGetFile(const char *concreteFilename, const char *abstractFilename, DFError **error)
{
    int r = 0;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFPackage *abstractPackage = DFPackageNewFilesystem(abstractPath,DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    concreteDoc = DFConcreteDocumentOpenFile(concreteFilename,error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error,"%s: %s",concreteFilename,DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractPackage);

    if (!DFGet(concreteDoc,abstractDoc,error) || (abstractDoc->htmlDoc == NULL)) {
        DFErrorFormat(error,"%s: %s",concreteFilename,DFErrorMessage(error));
        goto end;
    }

    if (DFFileExists(abstractFilename)) {
        DFErrorFormat(error,"%s: File already exists",abstractFilename);
        goto end;
    }

    if (!DFSerializeXMLFile(abstractDoc->htmlDoc,0,0,abstractFilename,error)) {
        DFErrorFormat(error,"%s: %s",abstractFilename,DFErrorMessage(error));
        goto end;
    }

    r = 1;

end:
    free(abstractPath);
    DFPackageRelease(abstractPackage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return r;
}

int DFPutFile(const char *concreteFilename, const char *abstractFilename, DFError **error)
{
    int ok = 0;
    DFDocument *htmlDoc2 = NULL;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFPackage *abstractPackage2 = DFPackageNewFilesystem(abstractPath,DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    htmlDoc2 = DFParseHTMLFile(abstractFilename,0,error);
    if (htmlDoc2 == NULL) {
        DFErrorFormat(error,"%s: %s",abstractFilename,DFErrorMessage(error));
        goto end;
    }

    concreteDoc = DFConcreteDocumentOpenFile(concreteFilename,error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error,"%s: %s",concreteFilename,DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractPackage2);
    abstractDoc->htmlDoc = DFDocumentRetain(htmlDoc2);

    ok = DFPut(concreteDoc,abstractDoc,error);

end:
    DFDocumentRelease(htmlDoc2);
    free(abstractPath);
    DFPackageRelease(abstractPackage2);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}

int DFCreateFile(const char *concreteFilename, const char *abstractFilename, DFError **error)
{
    int ok = 0;
    DFDocument *htmlDoc = NULL;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFPackage *abstractPackage = DFPackageNewFilesystem(abstractPath,DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    htmlDoc = DFParseHTMLFile(abstractFilename,0,error);
    if (htmlDoc == NULL) {
        DFErrorFormat(error,"%s: %s",abstractFilename,DFErrorMessage(error));
        goto end;
    }

    concreteDoc = DFConcreteDocumentCreateFile(concreteFilename,error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error,"%s: %s",concreteFilename,DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractPackage);
    abstractDoc->htmlDoc = DFDocumentRetain(htmlDoc);

    ok = DFCreate(concreteDoc,abstractDoc,error);

end:
    DFDocumentRelease(htmlDoc);
    free(abstractPath);
    DFPackageRelease(abstractPackage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}
