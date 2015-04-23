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

#ifndef DocFormats_ODFConverter_h
#define DocFormats_ODFConverter_h

#include "ODFPackage.h"
#include "DFXMLNames.h"
#include "DFBDT.h"
#include "DFDOM.h"
#include "DFClassNames.h"
#include <DocFormats/DFXMLForward.h>
#include "CSSSelector.h"
#include "CSSSheet.h"
#include "OOXMLTypedefs.h"
#include <DocFormats/DFStorage.h>
/*
#define EMUS_PER_POINT 12700

#define A4_WIDTH_TWIPS 11900
#define A4_HEIGHT_TWIPS 16840
#define LETTER_WIDTH_TWIPS 12240
#define LETTER_HEIGHT_TWIPS 15840

#define WORDREL_SETTINGS "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"
#define WORDREL_WEBSETTINGS "http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings"
#define WORDREL_FONTTABLE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable"
#define WORDREL_THEME "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme"
#define WORDREL_NUMBERING "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"
#define WORDREL_STYLES "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"
#define WORDREL_CORE_PROPERTIES "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"
#define WORDREL_EXTENDED_PROPERTIES "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties"
#define WORDREL_OFFICE_DOCUMENT "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"
#define WORDREL_THUMBNAIL "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail"
#define WORDREL_HYPERLINK "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink"
#define WORDREL_IMAGE "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"
#define WORDREL_FOOTNOTES "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes"
#define WORDREL_ENDNOTES "http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes"

#define WORDTYPE_SETTINGS "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"
#define WORDTYPE_WEBSETTINGS "application/vnd.openxmlformats-officedocument.wordprocessingml.webSettings+xml"
#define WORDTYPE_FONTTABLE "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"
#define WORDTYPE_THEME "application/vnd.openxmlformats-officedocument.theme+xml"
#define WORDTYPE_NUMBERING "application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"
#define WORDTYPE_STYLES "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"
#define WORDTYPE_CORE_PROPERTIES "application/vnd.openxmlformats-package.core-properties+xml"
#define WORDTYPE_EXTENDED_PROPERTIES "application/vnd.openxmlformats-officedocument.extended-properties+xml"
#define WORDTYPE_OFFICE_DOCUMENT "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"
#define WORDTYPE_FOOTNOTES "application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"
#define WORDTYPE_ENDNOTES "application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml"

int Word_isFigureParagraph(DFNode *p);
int Word_isEquationParagraph(DFNode *p);
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          ODFConverter                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ODFGetData {
    ODFConverter *conv;
};

struct ODFPutData {
    ODFConverter *conv;
    DFDocument *contentDoc;
    DFHashTable *numIdByHtmlId;
    DFHashTable *htmlIdByNumId;
};

struct ODFConverter {
    DFDocument *html;
    DFStorage *abstractStorage;
    char *idPrefix;
    ODFPackage *package;
    struct ODFSheet *styles;
    struct ODFNumbering *numbering;
    struct ODFTheme *theme;
    struct ODFSection *mainSection;
    struct ODFObjects *objects;
    struct ODFNoteGroup *footnotes;
    struct ODFNoteGroup *endnotes;
    DFHashTable *supportedContentTypes;
    DFBuffer *warnings;
    int haveFields;
    CSSSheet *styleSheet;
};

int ODFConverterGet(DFDocument *html, DFStorage *abstractStorage, ODFPackage *package, const char *idPrefix, DFError **error);
int ODFConverterPut(DFDocument *html, DFStorage *abstractStorage, ODFPackage *package, const char *idPrefix, DFError **error);
void ODFConverterWarning(ODFConverter *converter, const char *format, ...) ATTRIBUTE_FORMAT(printf,2,3);

char *ODFStyleIdForStyle(CSSStyle *style);
StyleFamily ODFStyleFamilyForSelector(const char *selector);

DFNode *ODFConverterCreateAbstract(ODFGetData *get, Tag tag, DFNode *concrete);
DFNode *ODFConverterGetConcrete(ODFPutData *put, DFNode *abstract);

void childrenToArray(DFNode *node, DFNode **children);
void replaceChildrenFromArray(DFNode *node, DFNode **children, Tag *tags);

#endif
