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
#include <DocFormats/Operations.h>
#include "DFFilesystem.h"
#include "DFString.h"
#include <DocFormats/DFStorage.h>
#include "Word.h"
#include "ODFText.h"
#include "DFHTML.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFZipFile.h"
#include <stdlib.h>

struct DFConcreteDocument {
    size_t retainCount;
    DFStorage *storage;
};

struct DFAbstractDocument {
    size_t retainCount;
    DFStorage *storage;
    DFDocument *htmlDoc;
};

DFConcreteDocument *DFConcreteDocumentNew(DFStorage *storage)
{
    DFConcreteDocument *concrete = (DFConcreteDocument *)calloc(1,sizeof(DFConcreteDocument));
    concrete->retainCount = 1;
    concrete->storage = DFStorageRetain(storage);
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
            DFStorage *storage = DFStorageCreateZip(filename,error);
            if (storage == NULL)
                return NULL;;
            DFConcreteDocument *concrete = DFConcreteDocumentNew(storage);
            DFStorageRelease(storage);
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
            DFStorage *storage = DFStorageOpenZip(filename,error);
            if (storage == NULL)
                return NULL;;
            DFConcreteDocument *concrete = DFConcreteDocumentNew(storage);
            DFStorageRelease(storage);
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

    DFStorageRelease(concrete->storage);
    free(concrete);
}

DFAbstractDocument *DFAbstractDocumentNew(DFStorage *storage)
{
    DFAbstractDocument *abstract = (DFAbstractDocument *)calloc(1,sizeof(DFAbstractDocument));
    abstract->retainCount = 1;
    abstract->storage = DFStorageRetain(storage);
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

    DFStorageRelease(abstract->storage);
    DFDocumentRelease(abstract->htmlDoc);
    free(abstract);
}

DFDocument *DFAbstractDocumentGetHTML(DFAbstractDocument *abstract)
{
    return abstract->htmlDoc;
}

void DFAbstractDocumentSetHTML(DFAbstractDocument *abstract, DFDocument *htmlDoc)
{
    DFDocumentRelease(abstract->htmlDoc);
    abstract->htmlDoc = DFDocumentRetain(htmlDoc);
}

int DFGet(DFConcreteDocument *concrete, DFAbstractDocument *abstract, DFError **error)
{
    if (DFStorageFormat(abstract->storage) != DFFileFormatHTML) {
        DFErrorFormat(error,"Abstract document must be in HTML format");
        return 0;
    }

    DFDocument *htmlDoc = NULL;
    switch (DFStorageFormat(concrete->storage)) {
        case DFFileFormatDocx:
            htmlDoc = WordGet(concrete->storage,abstract->storage,error);
            break;
        case DFFileFormatOdt:
            htmlDoc = ODFTextGet(concrete->storage,abstract->storage,error);
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
    if (DFStorageFormat(abstractDoc->storage) != DFFileFormatHTML) {
        DFErrorFormat(error,"Abstract document must be in HTML format");
        return 0;
    }

    int ok = 0;
    switch (DFStorageFormat(concreteDoc->storage)) {
        case DFFileFormatDocx:
            ok = WordPut(concreteDoc->storage,abstractDoc->storage,abstractDoc->htmlDoc,error);
            break;
        case DFFileFormatOdt:
            ok = ODFTextPut(concreteDoc->storage,abstractDoc->storage,abstractDoc->htmlDoc,error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }
    return ok;
}

int DFCreate(DFConcreteDocument *concreteDoc, DFAbstractDocument *abstractDoc, DFError **error)
{
    if (DFStorageFormat(abstractDoc->storage) != DFFileFormatHTML) {
        DFErrorFormat(error,"Abstract document must be in HTML format");
        return 0;
    }

    int ok = 0;
    switch (DFStorageFormat(concreteDoc->storage)) {
        case DFFileFormatDocx:
            ok = WordCreate(concreteDoc->storage,abstractDoc->storage,abstractDoc->htmlDoc,error);
            break;
        case DFFileFormatOdt:
            ok = ODFTextCreate(concreteDoc->storage,abstractDoc->storage,abstractDoc->htmlDoc,error);
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
    DFStorage *abstractStorage = DFStorageNewFilesystem(abstractPath,DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    concreteDoc = DFConcreteDocumentOpenFile(concreteFilename,error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error,"%s: %s",concreteFilename,DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractStorage);

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
    DFStorageRelease(abstractStorage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return r;
}

int DFPutFile(const char *concreteFilename, const char *abstractFilename, DFError **error)
{
    int ok = 0;
    DFDocument *htmlDoc2 = NULL;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFStorage *abstractStorage2 = DFStorageNewFilesystem(abstractPath,DFFileFormatHTML);
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

    abstractDoc = DFAbstractDocumentNew(abstractStorage2);
    abstractDoc->htmlDoc = DFDocumentRetain(htmlDoc2);

    ok = DFPut(concreteDoc,abstractDoc,error);

end:
    DFDocumentRelease(htmlDoc2);
    free(abstractPath);
    DFStorageRelease(abstractStorage2);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}

int DFCreateFile(const char *concreteFilename, const char *abstractFilename, DFError **error)
{
    int ok = 0;
    DFDocument *htmlDoc = NULL;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFStorage *abstractStorage = DFStorageNewFilesystem(abstractPath,DFFileFormatHTML);
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

    abstractDoc = DFAbstractDocumentNew(abstractStorage);
    abstractDoc->htmlDoc = DFDocumentRetain(htmlDoc);

    ok = DFCreate(concreteDoc,abstractDoc,error);

end:
    DFDocumentRelease(htmlDoc);
    free(abstractPath);
    DFStorageRelease(abstractStorage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}
