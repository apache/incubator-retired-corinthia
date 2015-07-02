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

#ifndef DocFormats_ODFManifest_h
#define DocFormats_ODFManifest_h

#include "DFDOM.h"
#include "DFHashTable.h"
#include "DFTypes.h"

/**

 \file

 # ODFManifest

 All ODF documents are required to contain a file called `META-INF/manifest.xml`. This file acts as
 as an index of all of the individual files within the zipped package, containing information such
 as their mime type. Every file in the zipped package must have an entry in the manifest, with
 the exception of the `META-INF/manifest.xml` file itself, and the `mimetype` file in the root of
 the package; both these files are explicitly forbidden to appear in the manifest.

 It is unclear exactly why the manifest is necessary when the only information it contains is the
 mimetype of the files (which could be easily deduced from their file extensions, if it is needed
 at all). Nonetheless, it is required by the spec, and OpenOffice refuses to open documents that
 do not contain a manifest. Cynics might suggest that it serves the purpose of making the file
 format more complicated.

 The manifest file has a root element called `manifest`, with child elements named `file-entry`.
 There is one `file-entry` element for each file in the package, including the root directory, which
 defines the mimetype of the docment as a whole. An example manifest file is given below:

     <mf:manifest xmlns:mf="urn:oasis:names:tc:opendocument:xmlns:mf:1.0" mf:version="1.2">
         <mf:file-entry mf:media-type="text/xml" mf:full-path="content.xml"/>
         <mf:file-entry mf:media-type="text/xml" mf:full-path="styles.xml"/>
         <mf:file-entry mf:media-type="text/xml" mf:full-path="settings.xml"/>
         <mf:file-entry mf:media-type="text/xml" mf:full-path="meta.xml"/>
     </mf:manifest>

 The ODFManifest class provides a C representation of the manifest, and primarily exists to ensure
 that all of the required entries are present when saving a file, thus ensuring the file is valid
 and can be successfully opened by OpenOffice. DocFormats itself does not use the manifest when
 reading the package, because it's entirely redundant - all the information it contains can be
 deduced from the directory hierarchy in the zip file.

 There are two ways to create a new manifest object, depending on whether you are creating a new
 ODF document, or opening an existing one. ODFManifestNew() will create a manifest with an empty
 document (save for the `manifest` root element). ODFManifestNewWithDoc() will create an object
 based on an existing document you have read from a file.

 The entriesByPath hash table maps from path names (strings) to `file-entry` elements. It is
 populated when you call ODFManifestNewWithDoc(). This hash table enables quick lookup of entries
 based on their path.

 */

typedef struct ODFManifest ODFManifest;

struct ODFManifest {
    size_t retainCount;
    DFDocument *doc;
    DFHashTable *entriesByPath;
};

ODFManifest *ODFManifestNew(void);
ODFManifest *ODFManifestNewWithDoc(DFDocument *doc);
ODFManifest *ODFManifestRetain(ODFManifest *manifest);
void ODFManifestRelease(ODFManifest *manifest);

void ODFManifestAddEntry(ODFManifest *manifest, const char *path, const char *mediaType,
                         const char *version);

#endif
