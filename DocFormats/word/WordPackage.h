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

#ifndef DocFormats_WordPackage_h
#define DocFormats_WordPackage_h

#include "DFXMLForward.h"
#include "OPC.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordPackage                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordPackage WordPackage;

struct WordPackage {
    size_t retainCount;
    char *tempPath;
    OPCPackage *opc;
    OPCPart *documentPart;
    OPCPart *numberingPart;
    OPCPart *stylesPart;
    OPCPart *settingsPart;
    OPCPart *themePart;
    OPCPart *footnotesPart;
    OPCPart *endnotesPart;
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

WordPackage *WordPackageNew(const char *tempPath);
WordPackage *WordPackageRetain(WordPackage *package);
void WordPackageRelease(WordPackage *package);

void WordPackageRemovePointlessElements(WordPackage *package);
const char *WordPackageTargetForDocumentRel(WordPackage *package, const char *relId);
void WordPackageSetPartsFromRels(WordPackage *package);
int WordPackageSimplifyFields(WordPackage *package);
void WordPackageCollapseBookmarks(WordPackage *package);
void WordPackageExpandBookmarks(WordPackage *package);

int WordPackageOpenNew(WordPackage *package, DFError **error);
int WordPackageOpenFrom(WordPackage *package, const char *filename, DFError **error);
int WordPackageSaveTo(WordPackage *package, const char *filename, DFError **error);
DFDocument *WordPackageGenerateHTML(WordPackage *package, const char *path, const char *idPrefix,
                                    DFError **error, DFBuffer *warnings);
int WordPackageUpdateFromHTML(WordPackage *package, DFDocument *input, const char *path,
                              const char *idPrefix, DFError **error, DFBuffer *warnings);

#endif
