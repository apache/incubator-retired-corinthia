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
#include "ODFPackage.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <string.h>

static DFDocument *createDocument(Tag rootTag, Tag childTag)
{
    DFDocument *doc = DFDocumentNewWithRoot(rootTag);
    DFCreateChildElement(doc->root,childTag);
    return doc;
}

static DFDocument *readDocument(ODFPackage *package, const char *filename, DFError **error)
{
    DFDocument *doc = DFParseXMLStorage(package->storage,filename,error);
    if (doc == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return NULL;
    }
    return doc;
}

static ODFManifest *readManifest(ODFPackage *package, DFError **error)
{
    DFDocument *manifestDoc = DFParseXMLStorage(package->storage,"META-INF/manifest.xml",error);
    if (manifestDoc == NULL) {
        DFErrorFormat(error,"META-INF/manifest.xml: %s",DFErrorMessage(error));
        return NULL;
    }
    return ODFManifestNewWithDoc(manifestDoc);
}

static int writeDocument(ODFPackage *package, DFDocument *doc, NamespaceID defaultNS,
                         const char *filename, DFError **error)
{
    int ok = DFSerializeXMLStorage(doc,defaultNS,0,package->storage,filename,error);
    if (!ok)
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
    return ok;
}

static int writeString(ODFPackage *package, const char *str, const char *filename, DFError **error)
{
    DFBuffer *buf = DFBufferNew();
    DFBufferAppendString(buf,str);
    int ok = DFBufferWriteToStorage(buf,package->storage,filename,error);
    DFBufferRelease(buf);

    if (!ok)
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
    return ok;
}

ODFPackage *ODFPackageOpenNew(DFStorage *storage, DFError **error)
{
    ODFPackage *package = (ODFPackage *)xcalloc(1,sizeof(ODFPackage));
    package->retainCount = 1;
    package->storage = DFStorageRetain(storage);

    // Create XML documents
    package->contentDoc = createDocument(OFFICE_DOCUMENT_CONTENT,OFFICE_BODY);
    package->metaDoc = createDocument(OFFICE_DOCUMENT_META,OFFICE_META);
    package->settingsDoc = createDocument(OFFICE_DOCUMENT_SETTINGS,OFFICE_SETTINGS);
    package->stylesDoc = createDocument(OFFICE_DOCUMENT_STYLES,OFFICE_STYLES);

    // Create manifst
    package->manifest = ODFManifestNew();
    ODFManifestAddEntry(package->manifest,"/","application/vnd.oasis.opendocument.text","1.2");
    ODFManifestAddEntry(package->manifest,"content.xml","text/xml",NULL);
    ODFManifestAddEntry(package->manifest,"meta.xml","text/xml",NULL);
    ODFManifestAddEntry(package->manifest,"settings.xml","text/xml",NULL);
    ODFManifestAddEntry(package->manifest,"styles.xml","text/xml",NULL);

    // Setup ODF objects
    package->sheet = ODFSheetNew(package->stylesDoc,package->contentDoc);

    return package;
}

ODFPackage *ODFPackageOpenFrom(DFStorage *storage, DFError **error)
{
    ODFPackage *package = (ODFPackage *)xcalloc(1,sizeof(ODFPackage));
    package->retainCount = 1;
    package->storage = DFStorageRetain(storage);

    // Read XML documents
    if ((package->contentDoc = readDocument(package,"content.xml",error)) == NULL)
        goto end;
    if ((package->metaDoc = readDocument(package,"meta.xml",error)) == NULL)
        goto end;
    if ((package->settingsDoc = readDocument(package,"settings.xml",error)) == NULL)
        goto end;
    if ((package->stylesDoc = readDocument(package,"styles.xml",error)) == NULL)
        goto end;

    // Read manifest
    if ((package->manifest = readManifest(package,error)) == NULL)
        goto end;

    // Setup ODF objects
    package->sheet = ODFSheetNew(package->stylesDoc,package->contentDoc);

    return package;

end:
    ODFPackageRelease(package);
    return NULL;
}

ODFPackage *ODFPackageRetain(ODFPackage *package)
{
    if (package != NULL)
        package->retainCount++;
    return package;
}

void ODFPackageRelease(ODFPackage *package)
{
    if ((package == NULL) || (--package->retainCount > 0))
        return;

    DFStorageRelease(package->storage);
    ODFManifestRelease(package->manifest);
    ODFSheetRelease(package->sheet);
    DFDocumentRelease(package->contentDoc);
    DFDocumentRelease(package->metaDoc);
    DFDocumentRelease(package->settingsDoc);
    DFDocumentRelease(package->stylesDoc);
    free(package);
}

int ODFPackageSave(ODFPackage *package, DFError **error)
{
    if (!writeDocument(package,package->contentDoc,NAMESPACE_NULL,"content.xml",error))
        return 0;
    if (!writeDocument(package,package->metaDoc,NAMESPACE_NULL,"meta.xml",error))
        return 0;
    if (!writeDocument(package,package->settingsDoc,NAMESPACE_NULL,"settings.xml",error))
        return 0;
    if (!writeDocument(package,package->stylesDoc,NAMESPACE_NULL,"styles.xml",error))
        return 0;
    if (!writeDocument(package,package->manifest->doc,NAMESPACE_NULL,"META-INF/manifest.xml",error))
        return 0;
    if (!writeString(package,"application/vnd.oasis.opendocument.text","mimetype",error))
        return 0;
    if (!DFStorageSave(package->storage,error))
        return 0;

    return 1;
}
