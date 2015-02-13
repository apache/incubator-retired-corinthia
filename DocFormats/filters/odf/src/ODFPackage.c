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

static DFDocument *readOrCreateDocument(ODFPackage *package, const char *filename, Tag rootTag, Tag childTag)
{
    char *fullPath = DFFormatString("%s/%s",package->tempPath,filename);
    DFDocument *doc = DFParseXMLFile(fullPath,NULL);
    free(fullPath);
    if (doc != NULL)
        return doc;

    doc = DFDocumentNewWithRoot(rootTag);
    DFCreateChildElement(doc->root,childTag);
    return doc;
}

static ODFManifest *readOrCreateManifest(ODFPackage *package)
{
    char *fullPath = DFFormatString("%s/%s",package->tempPath,"META-INF/manifest.xml");
    DFDocument *manifestDoc = DFParseXMLFile(fullPath,NULL);
    free(fullPath);
    return (manifestDoc != NULL) ? ODFManifestNewWithDoc(manifestDoc) : ODFManifestNew();
}

static int writeDocument(ODFPackage *package, DFDocument *doc, NamespaceID defaultNS, const char *filename, DFError **error)
{
    char *fullPath = DFFormatString("%s/%s",package->tempPath,filename);
    int ok = DFSerializeXMLFile(doc,defaultNS,0,fullPath,error);
    free(fullPath);

    if (!ok)
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
    return ok;
}

static int writeString(ODFPackage *package, const char *str, const char *filename, DFError **error)
{
    char *fullPath = DFFormatString("%s/%s",package->tempPath,filename);
    int ok = DFStringWriteToFile(str,fullPath,error);
    free(fullPath);

    if (!ok)
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
    return ok;
}

ODFPackage *ODFPackageNew(const char *tempPath, DFError **error)
{
    ODFPackage *package = (ODFPackage *)calloc(1,sizeof(ODFPackage));
    package->retainCount = 1;
    package->tempPath = strdup(tempPath);

    package->contentDoc = readOrCreateDocument(package,"content.xml",OFFICE_DOCUMENT_CONTENT,OFFICE_BODY);
    package->metaDoc = readOrCreateDocument(package,"meta.xml",OFFICE_DOCUMENT_META,OFFICE_META);
    package->settingsDoc = readOrCreateDocument(package,"settings.xml",OFFICE_DOCUMENT_SETTINGS,OFFICE_SETTINGS);
    package->stylesDoc = readOrCreateDocument(package,"styles.xml",OFFICE_DOCUMENT_STYLES,OFFICE_STYLES);

    package->manifest = readOrCreateManifest(package);
    ODFManifestAddEntry(package->manifest,"/","application/vnd.oasis.opendocument.text","1.2");
    ODFManifestAddEntry(package->manifest,"content.xml","text/xml",NULL);
    ODFManifestAddEntry(package->manifest,"meta.xml","text/xml",NULL);
    ODFManifestAddEntry(package->manifest,"settings.xml","text/xml",NULL);
    ODFManifestAddEntry(package->manifest,"styles.xml","text/xml",NULL);

    package->sheet = ODFSheetNew(package->stylesDoc,package->contentDoc);

    return package;
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

    free(package->tempPath);
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
    char *metaInfPath = DFFormatString("%s/META-INF",package->tempPath);
    if (!DFFileExists(metaInfPath) && !DFCreateDirectory(metaInfPath,1,error)) {
        free(metaInfPath);
        return 0;
    }
    free(metaInfPath);

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

    return 1;
}
