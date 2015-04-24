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

#ifndef DocFormats_WordPackage_h
#define DocFormats_WordPackage_h

#include <DocFormats/DFXMLForward.h>
#include "OPC.h"
#include <DocFormats/DFStorage.h>
#include "OOXMLTypedefs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordPackage                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordPackage {
    size_t retainCount;
    OPCPackage *opc;
    OPCPart *documentPart;
    DFDocument *document;
    DFDocument *numbering;
    DFDocument *styles;
    DFDocument *settings;
    DFDocument *theme;
    DFDocument *footnotes;
    DFDocument *endnotes;
};

void WordPackageSetDocument(WordPackage *package, DFDocument *document);
void WordPackageSetNumbering(WordPackage *package, DFDocument *numbering);
void WordPackageSetStyles(WordPackage *package, DFDocument *styles);
void WordPackageSetSettings(WordPackage *package, DFDocument *settings);
void WordPackageSetTheme(WordPackage *package, DFDocument *theme);
void WordPackageSetFootnotes(WordPackage *package, DFDocument *footnotes);
void WordPackageSetEndnotes(WordPackage *package, DFDocument *endnotes);

WordPackage *WordPackageRetain(WordPackage *package);
void WordPackageRelease(WordPackage *package);

const char *WordPackageTargetForDocumentRel(WordPackage *package, const char *relId);
int WordPackageSimplifyFields(WordPackage *package);
void WordPackageCollapseBookmarks(WordPackage *package);
void WordPackageExpandBookmarks(WordPackage *package);

WordPackage *WordPackageOpenNew(DFStorage *storage, DFError **error);
WordPackage *WordPackageOpenFrom(DFStorage *storage, DFError **error);
int WordPackageSave(WordPackage *package, DFError **error);

#endif
