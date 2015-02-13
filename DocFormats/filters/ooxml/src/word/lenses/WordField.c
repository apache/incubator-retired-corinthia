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
#include "WordField.h"
#include "WordLenses.h"
#include "WordBookmark.h"
#include "WordObjects.h"
#include "WordPackage.h"
#include "WordCaption.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFString.h"
#include "DFArray.h"
#include "DFCommon.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void WordFieldPut(WordPutData *put, DFNode *abstract, DFNode *concrete);

const char **Word_parseField(const char *str)
{
    size_t len = strlen(str);
    DFArray *components = DFArrayNew((DFCopyFunction)strdup,free);

    size_t start = 0;
    int inString = 0;
    for (size_t pos = 0; pos <= len; pos++) {
        if (inString) {
            if ((pos == len) || (str[pos] == '"')) {
                char *comp = DFSubstring(str,start,pos);
                DFArrayAppend(components,(char *)comp);
                free(comp);
                start = pos+1;
                inString = 0;
            }
        }
        else {
            if ((pos == len) || isspace(str[pos])) {
                if (pos > start) {
                    char *comp = DFSubstring(str,start,pos);
                    DFArrayAppend(components,(char *)comp);
                    free(comp);
                }
                start = pos+1;
            }
            else if (str[pos] == '"') {
                inString = 1;
                start = pos+1;
            }
        }
    }

    const char **result = DFStringArrayFlatten(components);
    DFArrayRelease(components);
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       DOM helper methods                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    DFNode *commonAncestor;
    DFNode *beginAncestor;
    DFNode *endAncestor;
} CommonAncestorInfo;

static CommonAncestorInfo findCommonAncestor(DFNode *beginNode, DFNode *endNode)
{
    CommonAncestorInfo info = { NULL, NULL, NULL };
    for (DFNode *beginA = beginNode; beginA != NULL; beginA = beginA->parent) {
        for (DFNode *endA = endNode; endA != NULL; endA = endA->parent) {
            if (beginA->parent == endA->parent) {
                info.commonAncestor = beginA->parent;
                info.beginAncestor = beginA;
                info.endAncestor = endA;
                return info;
            }
        }
    }
    return info;
}

static void removeNodes(DFNode *beginNode, DFNode *endNode)
{
    CommonAncestorInfo common = findCommonAncestor(beginNode,endNode);
    assert(common.commonAncestor != NULL);
    assert(common.beginAncestor != NULL);
    assert(common.endAncestor != NULL);

    DFNode *begin = beginNode;
    while (begin != common.beginAncestor) {
        DFNode *parent = begin->parent;
        if (begin->next != NULL)
            DFRemoveNode(begin->next);
        else
            begin = parent;
    }

    DFNode *end = endNode;
    while (end != common.endAncestor) {
        DFNode *parent = end->parent;
        if (end->prev != NULL)
            DFRemoveNode(end->prev);
        else
            end = parent;
    }

    if (common.beginAncestor != common.endAncestor) {
        while (common.beginAncestor->next != common.endAncestor)
            DFRemoveNode(common.beginAncestor->next);
    }

    while ((beginNode != NULL) && (beginNode->first == NULL) && (beginNode->tag != WORD_DOCUMENT)) {
        DFNode *parent = beginNode->parent;
        DFRemoveNode(beginNode);
        beginNode = parent;
    }

    while ((endNode != NULL) && (endNode->first == NULL) && (endNode->tag != WORD_DOCUMENT)) {
        DFNode *parent = endNode->parent;
        DFRemoveNode(endNode);
        endNode = parent;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       WordSimplification                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordSimplification {
    DFBuffer *instrText;
    DFNode *beginNode;
    DFNode *endNode;
    int haveFields;
    int inSeparate;
    int depth;
} WordSimplification;

static void replaceField(WordSimplification *simp)
{
    assert(simp->instrText != NULL);
    assert(simp->beginNode != NULL);
    assert(simp->endNode != NULL);

    if ((simp->beginNode->parent->tag == WORD_R) && (simp->endNode->parent->tag == WORD_R)) {
        DFNode *beginRun = simp->beginNode->parent;

        DFNode *simple = DFCreateElement(simp->beginNode->doc,WORD_FLDSIMPLE);
        DFSetAttribute(simple,WORD_INSTR,simp->instrText->data);
        DFInsertBefore(beginRun->parent,simple,beginRun);

        removeNodes(simp->beginNode,simp->endNode);
    }

    DFBufferRelease(simp->instrText);
    simp->instrText = NULL;
    simp->beginNode = NULL;
    simp->endNode = NULL;

    simp->haveFields = 1;
}

static void simplifyRecursive(WordSimplification *simp, DFNode *node)
{
    switch (node->tag) {
        case WORD_FLDCHAR: {
            const char *type = DFGetAttribute(node,WORD_FLDCHARTYPE);
            if (DFStringEquals(type,"begin")) {
                if (simp->depth == 0) {
                    DFBufferRelease(simp->instrText);
                    simp->instrText = DFBufferNew();
                    simp->beginNode = node;
                    simp->endNode = NULL;
                    simp->inSeparate = 0;
                }
                simp->depth++;
            }
            else if (DFStringEquals(type,"end") && (simp->depth > 0)) {
                simp->depth--;
                if (simp->depth == 0) {
                    simp->endNode = node;
                    replaceField(simp);
                }
            }
            else if (DFStringEquals(type,"separate")) {
                if (simp->depth == 1)
                    simp->inSeparate = 1;
            }
            break;
        }
        case WORD_INSTRTEXT: {
            if ((simp->depth == 1) && !simp->inSeparate) {
                char *value = DFNodeTextToString(node);
                DFBufferFormat(simp->instrText,"%s",value);
                free(value);
            }
            break;
        }
    }

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        simplifyRecursive(simp,child);
    }
}

int Word_simplifyFields(WordPackage *package)
{
    WordSimplification simp;
    bzero(&simp,sizeof(WordSimplification));
    simplifyRecursive(&simp,package->document->docNode);
    DFBufferRelease(simp.instrText);
    return simp.haveFields;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordFieldLens                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    WordRefUnknown,
    WordRefNum,
    WordRefText,
    WordRefDirection,
    WordRefCaptionText,
    WordRefLabelNum,
} WordRefType;

static const char *WordRefTypeClassName(WordRefType refType)
{
    switch (refType) {
        case WordRefText:
            return DFRefTextClass;
        case WordRefDirection:
            return DFRefDirectionClass;
        case WordRefCaptionText:
            return DFRefCaptionTextClass;
        case WordRefLabelNum:
            return DFRefLabelNumClass;
        case WordRefNum:
        default:
            return DFRefNumClass;
    }
}

static WordRefType WordRefTypeGet(const char **args, WordBookmark *bookmark)
{
    size_t argCount = DFStringArrayCount(args);
    WordRefType type = WordRefText;

    for (size_t argno = 2; argno < argCount; argno++) {
        const char *arg = args[argno];
        if (!strcmp(arg,"\\r"))
            type = WordRefNum; // Numbered reference (normal)
        else if (!strcmp(arg,"\\n"))
            type = WordRefNum; // Numbered reference (no context)
        else if (!strcmp(arg,"\\w"))
            type = WordRefNum; // Numbered reference (full context)
        else if (!strcmp(arg,"\\p"))
            type = WordRefDirection;
    }

    if ((bookmark->type == WordBookmarkTable) ||
        (bookmark->type == WordBookmarkFigure) ||
        (bookmark->type == WordBookmarkEquation)) {
        if (type == WordRefText) {
            DFNode *p = WordFindContainingParagraph(bookmark->element);
            if (p != NULL) {
                CaptionParts parts = WordBookmarkGetCaptionParts(bookmark);

                if (parts.beforeNum && !parts.num && !parts.afterNum)
                    type = WordRefCaptionText;
                else if (parts.beforeNum && parts.num && !parts.afterNum)
                    type = WordRefLabelNum;
            }
        }
    }

    return type;
}

static DFNode *WordFieldGet(WordGetData *get, DFNode *concrete)
{
    if (concrete->tag != WORD_FLDSIMPLE)
        return NULL;;

    const char *instr = DFGetAttribute(concrete,WORD_INSTR);
    if (instr != NULL) {
        const char **args = Word_parseField(instr);
        size_t argCount = DFStringArrayCount(args);

        if ((argCount >= 2) && !strcmp(args[0],"REF")) {
            WordBookmark *bookmark = WordObjectsBookmarkWithName(get->conv->objects,args[1]);
            if ((bookmark != NULL) && (bookmark->target != NULL)) {

                WordRefType type = WordRefTypeGet(args,bookmark);

                DFNode *a = WordConverterCreateAbstract(get,HTML_A,concrete);
                DFFormatAttribute(a,HTML_HREF,"#%s%u",get->conv->idPrefix,bookmark->target->seqNo);
                DFSetAttribute(a,HTML_CLASS,WordRefTypeClassName(type));

                free(args);
                return a;
            }
        }
        else if ((argCount >= 1) && !strcmp(args[0],"TOC")) {

            if ((argCount >= 2) && !strcmp(args[1],"\\o")) {
                DFNode *nav = WordConverterCreateAbstract(get,HTML_NAV,concrete);
                DFSetAttribute(nav,HTML_CLASS,DFTableOfContentsClass);
                free(args);
                return nav;
            }
            else if ((argCount >= 3) && !strcmp(args[1],"\\c")) {
                // FIXME: The names "Figure" and "Table" here will be different if the document
                // was created in a language other than English. We need to look through the
                // document to figure out which counter names are used in captions adjacent to
                // figures and tables to know what the counter names used in the document
                // actually are.

                // Another option might be just to collect a static list of names used in all the
                // major languages and base the detection on that. These would need to be checked
                // with multiple versions of word, as the names used could in theory change
                // between releases.

                // We should keep track of a set of "document parameters", which record the names
                // used for figure and table counters, as well as the prefixes used on numbered
                // figures and tables. The latter would correspond to the content property of the
                // caption::before and figcaption::before CSS rules.

                if (!strcmp(args[2],"Figure")) {
                    DFNode *nav = WordConverterCreateAbstract(get,HTML_NAV,concrete);
                    DFSetAttribute(nav,HTML_CLASS,DFListOfFiguresClass);
                    free(args);
                    return nav;
                }
                else if (!strcmp(args[2],"Table")) {
                    DFNode *nav = WordConverterCreateAbstract(get,HTML_NAV,concrete);
                    DFSetAttribute(nav,HTML_CLASS,DFListOfTablesClass);
                    free(args);
                    return nav;
                }
            }
        }

        DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
        DFSetAttribute(span,HTML_CLASS,DFFieldClass);
        DFNode *text = DFCreateTextNode(get->conv->html,instr);
        DFAppendChild(span,text);
        free(args);
        return span;
    }
    return NULL;
}

static int WordFieldIsVisible(WordPutData *put, DFNode *concrete)
{
    return 1;
}

static DFNode *WordFieldCreate(WordPutData *put, DFNode *abstract)
{
    DFNode *concrete = DFCreateElement(put->contentDoc,WORD_FLDSIMPLE);
    // fldSimple elements are required to have an instr attribute (even if it's empty), so set
    // it here in case update doesn't change it for some reason
    DFSetAttribute(concrete,WORD_INSTR,"");
    WordFieldPut(put,abstract,concrete);
    put->conv->haveFields = 1;
    return concrete;
}

static const char *bookmarkNameForHtmlId(WordConverter *converter, const char *htmlId, const char *refClass)
{
    DFNode *htmlElem = DFElementForIdAttr(converter->html,htmlId);
    if (htmlElem == NULL)
        return NULL;
    switch (htmlElem->tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6: {
            DFNode *labelSpan = htmlElem->first;
            if ((labelSpan == NULL) || (labelSpan->tag != HTML_SPAN))
                return NULL;;
            const char *labelClass = DFGetAttribute(labelSpan,HTML_CLASS);
            if (!DFStringEquals(labelClass,DFBookmarkClass))
                return NULL;
            return DFGetAttribute(labelSpan,WORD_NAME);
        }
        case HTML_FIGURE:
        case HTML_TABLE: {
            WordCaption *caption = WordObjectsCaptionForTarget(converter->objects,htmlElem);
            if (caption == NULL)
                return NULL;
            if (DFStringEquals(refClass,DFRefTextClass) && (caption->textBookmark != NULL))
                return caption->textBookmark->bookmarkName;
            else if (DFStringEquals(refClass,DFRefLabelNumClass) && (caption->labelNumBookmark != NULL))
                return caption->labelNumBookmark->bookmarkName;
            else if (DFStringEquals(refClass,DFRefCaptionTextClass) && (caption->captionTextBookmark != NULL))
                return caption->captionTextBookmark->bookmarkName;
            else if (caption->textBookmark != NULL)
                return caption->textBookmark->bookmarkName; // default is entire caption
        }
        default:
            return NULL;
    }
}

static void WordFieldPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (abstract->tag) {
        case HTML_SPAN: {
            const char *className = DFGetAttribute(abstract,HTML_CLASS);
            if (!DFStringEquals(className,DFFieldClass))
                return;
            char *text = DFNodeTextToString(abstract);
            DFSetAttribute(concrete,WORD_INSTR,text);
            free(text);
            break;
        }
        case HTML_A: {
            const char *href = DFGetAttribute(abstract,HTML_HREF);
            if ((href == NULL) || (href[0] != '#'))
                return;;

            const char *targetId = &href[1];
            const char *className = DFGetAttribute(abstract,HTML_CLASS);
            if (className == NULL)
                className = "";;
            const char *bookmarkName = bookmarkNameForHtmlId(put->conv,targetId,className);
            if (bookmarkName == NULL)
                return;;

            DFNode *htmlElem = DFElementForIdAttr(put->conv->html,targetId);
            if ((htmlElem != NULL) && ((htmlElem->tag == HTML_TABLE) || (htmlElem->tag == HTML_FIGURE))) {
                if (!DFStringEquals(className,DFRefTextClass) &&
                    !DFStringEquals(className,DFRefLabelNumClass) &&
                    !DFStringEquals(className,DFRefCaptionTextClass))
                    className = DFRefTextClass;
            }

            if (DFStringEquals(className,DFRefTextClass) ||
                DFStringEquals(className,DFRefLabelNumClass) ||
                DFStringEquals(className,DFRefCaptionTextClass))
                DFFormatAttribute(concrete,WORD_INSTR," REF %s \\h ",bookmarkName);
            else if (DFStringEquals(className,DFRefDirectionClass))
                DFFormatAttribute(concrete,WORD_INSTR," REF %s \\p \\h ",bookmarkName);
            else
                DFFormatAttribute(concrete,WORD_INSTR," REF %s \\r \\h ",bookmarkName);
            break;
        }
    }
}

WordLens WordFieldLens = {
    .isVisible = WordFieldIsVisible,
    .get = WordFieldGet,
    .put = WordFieldPut,
    .create = WordFieldCreate,
    .remove = NULL, // LENS FIXME
};
