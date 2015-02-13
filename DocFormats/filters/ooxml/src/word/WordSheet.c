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

#include "WordSheet.h"
#include "DFDOM.h"
#include "CSSSelector.h"
#include "DFString.h"
#include "Word.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static char *WordSheetIdentForType(const char *type, const char *styleId)
{
    return DFFormatString("%s/%s",type,styleId);
}

char *WordStyleNameToClassName(const char *name)
{
    char *className = strdup(name);
    for (char *c = className; *c != '\0'; c++) {
        if (*c == ' ')
            *c = '_';
    }
    return className;
}

char *WordStyleNameFromClassName(const char *name)
{
    char *className = strdup(name);
    for (char *c = className; *c != '\0'; c++) {
        if (*c == '_')
            *c = ' ';
    }
    return className;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordStyle                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static WordStyle *WordStyleNew(DFNode *element, const char *type, const char *styleId, const char *name)
{
    assert(element != NULL);
    assert(type != NULL);
    assert(styleId != NULL);
    assert(element->tag == WORD_STYLE);

    WordStyle *style = (WordStyle *)calloc(1,sizeof(WordStyle));
    style->retainCount = 1;
    style->element = element;
    style->type = (type != NULL) ? strdup(type) : NULL;
    style->styleId = (styleId != NULL) ? strdup(styleId) : NULL;
    style->ident = WordSheetIdentForType(style->type,style->styleId);
    style->basedOn = DFStrDup(DFGetChildAttribute(style->element,WORD_BASEDON,WORD_VAL));
    DFNode *pPr = DFChildWithTag(style->element,WORD_PPR);
    style->outlineLvl = DFStrDup(DFGetChildAttribute(pPr,WORD_OUTLINELVL,WORD_VAL));
    style->name = strdup(name);

    return style;
}

static WordStyle *WordStyleRetain(WordStyle *style)
{
    if (style != NULL)
        style->retainCount++;
    return style;
}

static void WordStyleRelease(WordStyle *style)
{
    if ((style == NULL) || (--style->retainCount > 0))
        return;

    free(style->type);
    free(style->styleId);
    free(style->selector);
    free(style->ident);
    free(style->basedOn);
    free(style->outlineLvl);
    free(style->name);
    free(style);
}

int WordStyleIsProtected(WordStyle *style)
{
    return (!strcmp(style->name,WordStyleNameFootnoteReference) ||
            !strcmp(style->name,WordStyleNameFootnoteText) ||
            !strcmp(style->name,WordStyleNameEndnoteReference) ||
            !strcmp(style->name,WordStyleNameEndnoteText));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordSheet                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordSheet {
    DFHashTable *stylesByIdent;
    DFHashTable *stylesByName;
    DFHashTable *stylesBySelector;
    DFDocument *doc;
};

static void determineSelectors(WordSheet *sheet);

WordSheet *WordSheetNew(DFDocument *doc)
{
    WordSheet *sheet = (WordSheet *)calloc(1,sizeof(WordSheet));

    sheet->stylesByIdent = DFHashTableNew((DFCopyFunction)WordStyleRetain,(DFFreeFunction)WordStyleRelease);
    sheet->stylesByName = DFHashTableNew((DFCopyFunction)WordStyleRetain,(DFFreeFunction)WordStyleRelease);
    sheet->stylesBySelector = DFHashTableNew((DFCopyFunction)WordStyleRetain,(DFFreeFunction)WordStyleRelease);
    sheet->doc = DFDocumentRetain(doc);
    if (sheet->doc == NULL)
        sheet->doc = DFDocumentNewWithRoot(WORD_STYLES);;

    DFNode *root = sheet->doc->root;
    for (DFNode *child = root->first; child != NULL; child = child->next) {
        if (child->tag == WORD_STYLE) {
            const char *type = DFGetAttribute(child,WORD_TYPE);
            const char *styleId = DFGetAttribute(child,WORD_STYLEID);
            const char *name = DFGetChildAttribute(child,WORD_NAME,WORD_VAL);
            if ((type != NULL) && (styleId != NULL) && (name != NULL)) {
                WordStyle *style = WordStyleNew(child,type,styleId,name);
                DFHashTableAdd(sheet->stylesByIdent,style->ident,style);
                DFHashTableAdd(sheet->stylesByName,style->name,style);
                WordStyleRelease(style);
            }
        }
    }
    determineSelectors(sheet);

    return sheet;
}

void WordSheetFree(WordSheet *sheet)
{
    DFHashTableRelease(sheet->stylesByIdent);
    DFHashTableRelease(sheet->stylesByName);
    DFHashTableRelease(sheet->stylesBySelector);
    DFDocumentRelease(sheet->doc);
    free(sheet);
}

const char **WordSheetCopyIdents(WordSheet *sheet)
{
    return DFHashTableCopyKeys(sheet->stylesByIdent);
}

WordStyle *WordSheetStyleForIdent(WordSheet *sheet, const char *ident)
{
    return DFHashTableLookup(sheet->stylesByIdent,ident);
}

WordStyle *WordSheetStyleForTypeId(WordSheet *sheet, const char *type, const char *styleId)
{
    if ((type == NULL) || (styleId == NULL))
        return NULL;
    char *ident = WordSheetIdentForType(type,styleId);
    WordStyle *style = WordSheetStyleForIdent(sheet,ident);
    free(ident);
    return style;
}

WordStyle *WordSheetStyleForName(WordSheet *sheet, const char *name)
{
    if (name == NULL)
        return NULL;
    return DFHashTableLookup(sheet->stylesByName,name);
}

WordStyle *WordSheetStyleForSelector(WordSheet *sheet, const char *selector)
{
    if (selector == NULL)
        return NULL;
    return DFHashTableLookup(sheet->stylesBySelector,selector);
}

WordStyle *WordSheetAddStyle(WordSheet *sheet, const char *type, const char *styleId, const char *name, const char *selector)
{
    assert(type != NULL);
    assert(styleId != NULL);
    assert(name != NULL);
    assert(selector != NULL);

    DFNode *element = DFCreateChildElement(sheet->doc->root,WORD_STYLE);
    DFSetAttribute(element,WORD_STYLEID,styleId);
    DFSetAttribute(element,WORD_TYPE,type);
    DFAppendChild(sheet->doc->root,element);
    DFNode *nameNode = DFCreateChildElement(element,WORD_NAME);
    DFSetAttribute(nameNode,WORD_VAL,name);

    WordStyle *style = WordStyleNew(element,type,styleId,name);
    style->selector = strdup(selector);
    DFHashTableAdd(sheet->stylesByIdent,style->ident,style);
    DFHashTableAdd(sheet->stylesByName,style->name,style);
    DFHashTableAdd(sheet->stylesBySelector,style->selector,style);
    WordStyleRelease(style);
    return style;
}

void WordSheetRemoveStyle(WordSheet *sheet, WordStyle *style)
{
    WordStyleRetain(style);
    DFRemoveNode(style->element);
    DFHashTableRemove(sheet->stylesByIdent,style->ident);
    DFHashTableRemove(sheet->stylesByName,style->name);
    if (style->selector != NULL)
        DFHashTableRemove(sheet->stylesBySelector,style->selector);
    WordStyleRelease(style);
}

const char *WordSheetNameForStyleId(WordSheet *sheet, const char *type, const char *styleId)
{
    WordStyle *style = WordSheetStyleForTypeId(sheet,type,styleId);
    return (style != NULL) ? style->name : NULL;
}

const char *WordSheetSelectorForStyleId(WordSheet *sheet, const char *type, const char *styleId)
{
    if ((type == NULL) || (styleId == NULL))
        return NULL;;

    WordStyle *style = WordSheetStyleForTypeId(sheet,type,styleId);
    if (style == NULL)
        return NULL;

    return style->selector;
}

const char *WordSheetStyleIdForSelector(WordSheet *sheet, const char *selector)
{
    if (selector == NULL)
        return NULL;;

    WordStyle *style = WordSheetStyleForSelector(sheet,selector);
    if (style == NULL)
        return NULL;

    return style->styleId;
}

static void determineSelectors(WordSheet *sheet)
{
    const char **allIdents = DFHashTableCopyKeys(sheet->stylesByIdent);
    for (int i = 0; allIdents[i]; i++) {
        const char *ident = allIdents[i];
        WordStyle *style = DFHashTableLookup(sheet->stylesByIdent,ident);
        const char *outlineLvl = NULL;

        // Compute inherited properties
        WordStyle *ancestor = style;
        DFHashTable *visited = DFHashTableNew((DFCopyFunction)strdup,free);
        while ((ancestor != NULL) && (DFHashTableLookup(visited,ancestor->ident) == NULL)) {
            DFHashTableAdd(visited,ancestor->ident,"");

            if (outlineLvl == NULL)
                outlineLvl = ancestor->outlineLvl;

            if (ancestor->basedOn != NULL)
                ancestor = WordSheetStyleForTypeId(sheet,ancestor->type,ancestor->basedOn);
            else
                ancestor = NULL;
        }
        DFHashTableRelease(visited);

        char *name = (style->name != NULL) ? WordStyleNameToClassName(style->name) : WordStyleNameToClassName(style->styleId);

        if (DFStringEquals(style->type,"paragraph")) {
            if ((outlineLvl != NULL) && (atoi(outlineLvl) >= 0) && (atoi(outlineLvl) <= 5)) {
                int headingLevel = atoi(outlineLvl) + 1;
                switch (headingLevel) {
                    case 1: style->selector = CSSMakeSelector("h1",name); break;
                    case 2: style->selector = CSSMakeSelector("h2",name); break;
                    case 3: style->selector = CSSMakeSelector("h3",name); break;
                    case 4: style->selector = CSSMakeSelector("h4",name); break;
                    case 5: style->selector = CSSMakeSelector("h5",name); break;
                    case 6: style->selector = CSSMakeSelector("h6",name); break;
                    default: style->selector = CSSMakeSelector("p",name); break;
                }
            }
            else if (DFStringEquals(style->styleId,"Caption")) {
                style->selector = CSSMakeSelector("caption",NULL);
            }
            else if (DFStringEquals(style->styleId,"Figure")) {
                style->selector = CSSMakeSelector("figure",NULL);
            }
            else {
                style->selector = CSSMakeSelector("p",name);
            }
        }
        else if (DFStringEquals(style->type,"character")) {
            style->selector = CSSMakeSelector("span",name);
        }
        else if (DFStringEquals(style->type,"table")) {
            style->selector = CSSMakeSelector("table",name);
        }
        if (style->selector != NULL)
            DFHashTableAdd(sheet->stylesBySelector,style->selector,style);
        free(name);
    }
    free(allIdents);
}

static void setupNoteReferenceStyle(WordStyle *style)
{
    free(style->selector);
    style->selector = CSSMakeSelector("span",style->styleId);
    // FIXME: Set basedOn
    DFNode *rPr = DFCreateChildElement(style->element,WORD_RPR);
    DFNode *vertAlign = DFCreateChildElement(rPr,WORD_VERTALIGN);
    DFSetAttribute(vertAlign,WORD_VAL,"superscript");
}

static void setupNoteTextStyle(WordStyle *style)
{
    free(style->selector);
    style->selector = CSSMakeSelector("p",style->styleId);
    // FIXME: Set basedOn
}

WordStyle *WordSheetFootnoteReference(WordSheet *sheet)
{
    WordStyle *style = WordSheetStyleForName(sheet,WordStyleNameFootnoteReference);
    if (style != NULL)
        return style;

    style = WordSheetAddStyle(sheet,"character","FootnoteReference",WordStyleNameFootnoteReference,"span.FootnoteReference");
    setupNoteReferenceStyle(style);
    return style;
}

WordStyle *WordSheetFootnoteText(WordSheet *sheet)
{
    WordStyle *style = WordSheetStyleForName(sheet,WordStyleNameFootnoteText);
    if (style != NULL)
        return style;

    style = WordSheetAddStyle(sheet,"paragraph","FootnoteText",WordStyleNameFootnoteText,"p.FootnoteText");
    setupNoteTextStyle(style);
    return style;
}

WordStyle *WordSheetEndnoteReference(WordSheet *sheet)
{
    WordStyle *style = WordSheetStyleForName(sheet,WordStyleNameEndnoteReference);
    if (style != NULL)
        return style;

    style = WordSheetAddStyle(sheet,"character","EndnoteReference",WordStyleNameEndnoteReference,"span.EndnoteReference");
    setupNoteReferenceStyle(style);
    return style;
}

WordStyle *WordSheetEndnoteText(WordSheet *sheet)
{
    WordStyle *style = WordSheetStyleForName(sheet,WordStyleNameEndnoteText);
    if (style != NULL)
        return style;

    style = WordSheetAddStyle(sheet,"paragraph","EndnoteText",WordStyleNameEndnoteText,"p.EndnoteText");
    setupNoteTextStyle(style);
    return style;
}
