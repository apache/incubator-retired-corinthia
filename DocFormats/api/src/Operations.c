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
#include "ODF.h"
#include "DFHTML.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFZipFile.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct DFConcreteDocument {
    size_t retainCount;
    DFStorage *storage;
};

struct DFAbstractDocument {
    size_t retainCount;
    DFStorage *storage;
    DFDocument *htmlDoc;
};

/**
 * Compute a hash of the set of all files in the archive. When the get operation is executed,
 * this hash is stored in the HTML file, as a record of the document from which it was generated.
 * When the put operation is executed, the hash is compared with that of the HTML file, and an error
 * reported if a mismatch occurs.
 *
 * This check ensures that put can only be executed with HTML documents that were genuinely
 * generated from this exact (version of the) document, and thus can be safely assumed to have id
 * attributes that correctly match elements in the HTML document to elements in the original XML
 * file(s) from which they were generated, avoiding corruption during the update process.
 *
 * If someone tries to call put with a HTML document that was not originally created from this exact
 * concrete document, the operation will fail.
 */
static int computeDocumentHash(DFStorage *storage, DFHashCode *result, DFError **error)
{
    int ok = 0;
    *result = 0;

    DFHashCode hash = 0;
    DFHashBegin(hash);
    const char **filenames = DFStorageList(storage,error);
    if (filenames == NULL)
        goto end;
    DFSortStringsCaseSensitive(filenames);
    for (int i = 0; filenames[i]; i++) {
        unsigned char *buf = NULL;
        size_t nbytes = 0;
        if (!DFStorageRead(storage,filenames[i],(void **)&buf,&nbytes,error)) {
            DFErrorFormat(error,"%s: %s",filenames[i],DFErrorMessage(error));
            goto end;
        }
        // The hash algorithm works on 32-bit integers; add 4 NULL bytes at the end of the buffer to
        // ensure its entire contents are taken into account when computing the hash.
        buf = xrealloc(buf,nbytes+4);
        memset(&buf[nbytes],0,4);
        uint32_t *intbuf = (uint32_t *)buf;
        for (size_t pos = 0; pos < (nbytes+3)/4; pos++)
            DFHashUpdate(hash,intbuf[pos]);
        free(buf);
    }
    DFHashEnd(hash);
    *result = hash;
    ok = 1;

end:
    free(filenames);
    return ok;
}

DFConcreteDocument *DFConcreteDocumentNew(DFStorage *storage)
{
    DFConcreteDocument *concrete = 
      (DFConcreteDocument *)xcalloc(1,sizeof(DFConcreteDocument));
    concrete->retainCount = 1;
    concrete->storage = DFStorageRetain(storage);
    return concrete;
}

DFConcreteDocument
*DFConcreteDocumentCreateFile(const char *filename, DFError **error)
{
    DFFileFormat format = DFFileFormatFromFilename(filename);
    switch (format) {
        case DFFileFormatDocx:
        case DFFileFormatXlsx:
        case DFFileFormatPptx:
        case DFFileFormatOdt:
        case DFFileFormatOds:
        case DFFileFormatOdp: {
            DFStorage *storage = DFStorageCreateZip(filename, error);
            if (storage == NULL)
                return NULL;;
            DFConcreteDocument *concrete =
              DFConcreteDocumentNew(storage);
            DFStorageRelease(storage);
            return concrete;
        }
        default:
            DFErrorFormat(error,
                          "Unsupported format for "
                          "DFConcreteDocumentCreateFile");
            return NULL;
    }
}

DFConcreteDocument 
*DFConcreteDocumentOpenFile(const char *filename, DFError **error)
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
            DFConcreteDocument *concrete =
              DFConcreteDocumentNew(storage);
            DFStorageRelease(storage);
            return concrete;
        }
        default:
            DFErrorFormat(error,"Unsupported format for" 
                          "DFConcreteDocumentCreateFile");
            return NULL;
    }
}

DFConcreteDocument 
*DFConcreteDocumentRetain(DFConcreteDocument *concrete)
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
    DFAbstractDocument *abstract =
      (DFAbstractDocument *)xcalloc(1,sizeof(DFAbstractDocument));
    abstract->retainCount = 1;
    abstract->storage = DFStorageRetain(storage);
    return abstract;
}

DFAbstractDocument
*DFAbstractDocumentRetain(DFAbstractDocument *abstract)
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

void DFAbstractDocumentSetHTML(DFAbstractDocument *abstract,
                               DFDocument *htmlDoc)
{
    DFDocumentRelease(abstract->htmlDoc);
    abstract->htmlDoc = DFDocumentRetain(htmlDoc);
}

int DFGet(DFConcreteDocument *concrete,
          DFAbstractDocument *abstract,
          DFError **error)
{
    if (DFStorageFormat(abstract->storage) != DFFileFormatHTML) {
        DFErrorFormat(error,
                      "Abstract document must be in HTML format");
        return 0;
    }

    DFHashCode hash = 0;
    if (!computeDocumentHash(concrete->storage,&hash,error))
        return 0;
    char hashstr[100];
    snprintf(hashstr,100,"%X",hash);

    char hashprefix[100];
    snprintf(hashprefix,100,"%s-",hashstr);
    const char *idPrefix;
    if (DFStorageExists(abstract->storage,"test-mode"))
        idPrefix = NULL;
    else
        idPrefix = hashprefix;

    DFDocument *htmlDoc = NULL;
    switch (DFStorageFormat(concrete->storage)) {
        case DFFileFormatDocx:
            htmlDoc = WordGet(concrete->storage,
                              abstract->storage,
                              idPrefix,
                              error);
            break;
        case DFFileFormatOdt:
            htmlDoc = ODFGet(concrete->storage,
                             abstract->storage,
                             idPrefix,
                             error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }

    if (htmlDoc == NULL)
        return 0;;

    // Store a hash of the concrete document in the HTML file, so we can check it in DFPut()
    HTMLMetaSet(htmlDoc,"corinthia-document-hash",hashstr);

    DFDocumentRelease(abstract->htmlDoc);
    abstract->htmlDoc = htmlDoc;
    return 1;
}

int DFPut(DFConcreteDocument *concreteDoc,
          DFAbstractDocument *abstractDoc,
          DFError **error)
{
    if (DFStorageFormat(abstractDoc->storage) != DFFileFormatHTML) {
        DFErrorFormat(error,
                      "Abstract document must be in HTML format");
        return 0;
    }

    // Check that the document hash in the HTML file matches that of the concrete document. This
    // ensures that we're using a HTML file that was generated from this exact document (see above)
    // and can rely on the element mappings from the id attributes. This comparison is ignored
    // for test cases, which specify the special value "ignore" in the meta tag.
    DFHashCode expectedHash = 0;
    if (!computeDocumentHash(concreteDoc->storage,&expectedHash,error))
        return 0;;
    DFHashCode actualHash = 0;
    int hashMatches = 0;
    const char *hashstr = HTMLMetaGet(abstractDoc->htmlDoc,"corinthia-document-hash");
    if ((hashstr != NULL) && (sscanf(hashstr,"%X",&actualHash) == 1))
        hashMatches = (expectedHash == actualHash);
    if (!hashMatches && !DFStringEquals(hashstr,"ignore")) {
        DFErrorFormat(error,"HTML document was generated from a different file to the one being updated");
        return 0;
    }

    char hashprefix[100];
    snprintf(hashprefix,100,"%s-",hashstr);
    const char *idPrefix;
    if (DFStringEquals(hashstr,"ignore"))
        idPrefix = NULL;
    else
        idPrefix = hashprefix;

    int ok = 0;
    switch (DFStorageFormat(concreteDoc->storage)) {
        case DFFileFormatDocx:
            ok = WordPut(concreteDoc->storage,
                         abstractDoc->storage,
                         abstractDoc->htmlDoc,
                         idPrefix,
                         error);
            break;
        case DFFileFormatOdt:
            ok = ODFPut(concreteDoc->storage,
                        abstractDoc->storage,
                        abstractDoc->htmlDoc,
                        idPrefix,
                        error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }
    return ok;
}

int DFCreate(DFConcreteDocument *concreteDoc,
             DFAbstractDocument *abstractDoc,
             DFError **error)
{
    if (DFStorageFormat(abstractDoc->storage) != DFFileFormatHTML) {
        DFErrorFormat(error,
                      "Abstract document must be in HTML format");
        return 0;
    }

    int ok = 0;
    switch (DFStorageFormat(concreteDoc->storage)) {
        case DFFileFormatDocx:
            ok = WordCreate(concreteDoc->storage,
                            abstractDoc->storage,
                            abstractDoc->htmlDoc,
                            error);
            break;
        case DFFileFormatOdt:
            ok = ODFCreate(concreteDoc->storage,
                               abstractDoc->storage,
                               abstractDoc->htmlDoc,
                               error);
            break;
        default:
            DFErrorFormat(error,"Unsupported file format");
            break;
    }
    return ok;
}

int DFGetFile(const char *concreteFilename,
              const char *abstractFilename,
              DFError **error)
{
    int ok = 0;

    if (DFFileExists(abstractFilename)) {
        DFErrorFormat(error,
                      "%s: File already exists",
                      abstractFilename);
        return ok;
    }

    char *abstractPath = DFPathDirName(abstractFilename);
    DFStorage *abstractStorage =
      DFStorageNewFilesystem(abstractPath, DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    concreteDoc = DFConcreteDocumentOpenFile(concreteFilename, error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error, "%s: %s",
                      concreteFilename,
                      DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractStorage);

    if (!DFGet(concreteDoc, abstractDoc, error)
        || (abstractDoc->htmlDoc == NULL)) {
        DFErrorFormat(error, "%s: %s",
                      concreteFilename,
                      DFErrorMessage(error));
        goto end;
    }

    if (!DFSerializeXMLFile(abstractDoc->htmlDoc,
                            0, 0,
                            abstractFilename,error)) {
        DFErrorFormat(error, "%s: %s",
                      abstractFilename,
                      DFErrorMessage(error));
        goto end;
    }

    ok = 1;

end:
    free(abstractPath);
    DFStorageRelease(abstractStorage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}

int DFPutFile(const char *concreteFilename,
              const char *abstractFilename,
              DFError **error)
{
    int ok = 0;
    DFDocument *htmlDoc2 = NULL;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFStorage *abstractStorage2 =
      DFStorageNewFilesystem(abstractPath, DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    htmlDoc2 = DFParseHTMLFile(abstractFilename, 0, error);
    if (htmlDoc2 == NULL) {
        DFErrorFormat(error,"%s: %s",
                      abstractFilename,
                      DFErrorMessage(error));
        goto end;
    }

    concreteDoc = DFConcreteDocumentOpenFile(concreteFilename, error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error, "%s: %s",
                      concreteFilename,
                      DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractStorage2);
    abstractDoc->htmlDoc = DFDocumentRetain(htmlDoc2);

    ok = DFPut(concreteDoc, abstractDoc, error);

end:
    DFDocumentRelease(htmlDoc2);
    free(abstractPath);
    DFStorageRelease(abstractStorage2);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}

int DFCreateFile(const char *concreteFilename,
                 const char *abstractFilename,
                 DFError **error)
{
    int ok = 0;
    DFDocument *htmlDoc = NULL;
    char *abstractPath = DFPathDirName(abstractFilename);
    DFStorage *abstractStorage =
      DFStorageNewFilesystem(abstractPath, DFFileFormatHTML);
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    htmlDoc = DFParseHTMLFile(abstractFilename, 0, error);
    if (htmlDoc == NULL) {
        DFErrorFormat(error,"%s: %s",
                      abstractFilename,
                      DFErrorMessage(error));
        goto end;
    }

    concreteDoc =
      DFConcreteDocumentCreateFile(concreteFilename, error);
    if (concreteDoc == NULL) {
        DFErrorFormat(error, "%s: %s",
                      concreteFilename,
                      DFErrorMessage(error));
        goto end;
    }

    abstractDoc = DFAbstractDocumentNew(abstractStorage);
    abstractDoc->htmlDoc = DFDocumentRetain(htmlDoc);

    ok = DFCreate(concreteDoc, abstractDoc, error);

end:
    DFDocumentRelease(htmlDoc);
    free(abstractPath);
    DFStorageRelease(abstractStorage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}
