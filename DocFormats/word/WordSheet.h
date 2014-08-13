//
//  WordSheet.h
//  DocFormats
//
//  Created by Peter Kelly on 15/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordSheet_h
#define DocFormats_WordSheet_h

#include "DFXMLForward.h"
#include "DFHashTable.h"
#include "DFTypes.h"

char *WordStyleNameToClassName(const char *name);
char *WordStyleNameFromClassName(const char *name);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordStyle                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordStyle WordStyle;

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

typedef struct WordSheet WordSheet;

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
