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
#include "ODFManifest.h"
#include "DFXMLNames.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <string.h>

static void ODFManifestParse(ODFManifest *manifest)
{
    for (DFNode *child = manifest->doc->root; child != NULL; child = child->next) {
        if (child->tag == MF_FILE_ENTRY) {
            const char *path = DFGetAttribute(child,MF_FULL_PATH);
            if (path != NULL)
                DFHashTableAdd(manifest->entriesByPath,path,child);
        }
    }
}

ODFManifest *ODFManifestNew(void)
{
    ODFManifest *manifest = (ODFManifest *)calloc(1,sizeof(ODFManifest));
    manifest->retainCount = 1;
    manifest->doc = DFDocumentNewWithRoot(MF_MANIFEST);
    manifest->entriesByPath = DFHashTableNew(NULL,NULL);
    return manifest;
}

ODFManifest *ODFManifestNewWithDoc(DFDocument *doc)
{
    ODFManifest *manifest = (ODFManifest *)calloc(1,sizeof(ODFManifest));
    manifest->doc = DFDocumentRetain(doc);
    manifest->entriesByPath = DFHashTableNew(NULL,NULL);
    ODFManifestParse(manifest);
    return manifest;
}

ODFManifest *ODFManifestRetain(ODFManifest *manifest)
{
    if (manifest != NULL)
        manifest->retainCount++;
    return manifest;
}

void ODFManifestRelease(ODFManifest *manifest)
{
    if ((manifest == NULL) || (--manifest->retainCount > 0))
        return;

    DFDocumentRelease(manifest->doc);
    DFHashTableRelease(manifest->entriesByPath);
    free(manifest);
}

void ODFManifestAddEntry(ODFManifest *manifest, const char *path, const char *mediaType,
                         const char *version)
{
    DFNode *entry = DFHashTableLookup(manifest->entriesByPath,path);
    if (entry == NULL) {
        entry = DFCreateChildElement(manifest->doc->root,MF_FILE_ENTRY);
        DFHashTableAdd(manifest->entriesByPath,path,entry);
    }
    DFSetAttribute(entry,MF_FULL_PATH,path);
    DFSetAttribute(entry,MF_MEDIA_TYPE,mediaType);
    DFSetAttribute(entry,MF_VERSION,version);
    if (!strcmp(path,"") && (version != NULL))
        DFSetAttribute(manifest->doc->root,MF_VERSION,version);
}
