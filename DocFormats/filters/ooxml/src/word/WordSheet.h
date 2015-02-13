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

#ifndef DocFormats_WordSheet_h
#define DocFormats_WordSheet_h

#include <DocFormats/DFXMLForward.h>
#include "DFHashTable.h"
#include "DFTypes.h"
#include "OOXMLTypedefs.h"

char *WordStyleNameToClassName(const char *name);
char *WordStyleNameFromClassName(const char *name);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordStyle                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordStyle {
    size_t retainCount;
    DFNode *element;
    char *type;
    char *styleId;
    char *selector;
    char *ident;
    char *basedOn;
    char *outlineLvl;
    char *name;
};

int WordStyleIsProtected(WordStyle *style);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordSheet                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordSheet *WordSheetNew(DFDocument *doc);
void WordSheetFree(WordSheet *sheet);
const char **WordSheetCopyIdents(WordSheet *sheet);
WordStyle *WordSheetStyleForIdent(WordSheet *sheet, const char *ident);
WordStyle *WordSheetStyleForTypeId(WordSheet *sheet, const char *type, const char *styleId);
WordStyle *WordSheetStyleForName(WordSheet *sheet, const char *name);
WordStyle *WordSheetStyleForSelector(WordSheet *sheet, const char *selector);
WordStyle *WordSheetAddStyle(WordSheet *sheet, const char *type, const char *styleId, const char *name, const char *selector);
void WordSheetRemoveStyle(WordSheet *sheet, WordStyle *style);
const char *WordSheetNameForStyleId(WordSheet *sheet, const char *type, const char *styleId);
const char *WordSheetSelectorForStyleId(WordSheet *sheet, const char *type, const char *styleId);
const char *WordSheetStyleIdForSelector(WordSheet *sheet, const char *selector);

WordStyle *WordSheetFootnoteReference(WordSheet *sheet);
WordStyle *WordSheetFootnoteText(WordSheet *sheet);
WordStyle *WordSheetEndnoteReference(WordSheet *sheet);
WordStyle *WordSheetEndnoteText(WordSheet *sheet);

#endif
