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
#include "WordLists.h"
#include "DFDOM.h"
#include "CSSProperties.h"
#include "CSSLength.h"
#include "WordConverter.h"
#include "WordNumbering.h"
#include "WordStyles.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct ListDimensions {
    double marginLeftPct;
    double textIndentPct;
    double totalPct;
} ListDimensions;

static ListDimensions ListDimensionsZero = { 0, 0, 0 };

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ListFrame                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ListFrame ListFrame;

struct ListFrame {
    DFNode *element;
    ListFrame *parent;
    int numId;
    int ilvl;
    ListDimensions dimensions;
};

ListFrame *ListFrameNew(DFNode *element, ListFrame *parent, int numId, int ilvl, ListDimensions dimensions)
{
    ListFrame *frame = (ListFrame *)calloc(1,sizeof(ListFrame));
    frame->element = element;
    frame->parent = parent;
    frame->numId = numId;
    frame->ilvl = ilvl;
    frame->dimensions = dimensions;
    return frame;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ListStack                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    ListFrame *top;
} ListStack;

static ListFrame *ListStackPushFrame(ListStack *stack, DFNode *element, int numId, int ilvl, ListDimensions dimensions)
{
    stack->top = ListFrameNew(element, stack->top, numId, ilvl, dimensions);
    return stack->top;
}

static void ListStackPop(ListStack *stack)
{
    if (stack->top != NULL) {
        ListFrame *oldTop = stack->top;
        stack->top = stack->top->parent;
        free(oldTop);
    }
}

static void ListStackPopToAboveIlvl(ListStack *stack, int ilvl)
{
    while ((stack->top != NULL) && (stack->top->ilvl >= ilvl))
        ListStackPop(stack);
}

//@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordLists                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static ListDimensions cssPropertiesIndent(CSSProperties *properties)
{
    ListDimensions result;
    result.marginLeftPct = 0;
    result.textIndentPct = 0;
    result.totalPct = 0;

    if (CSSGet(properties,"margin-left") != NULL) {
        CSSLength length = CSSLengthFromString(CSSGet(properties,"margin-left"));
        if (CSSLengthIsValid(length) && (length.units == UnitsPct))
            result.marginLeftPct = length.value;
    }

    if (CSSGet(properties,"text-indent") != NULL) {
        CSSLength length = CSSLengthFromString(CSSGet(properties,"text-indent"));
        if (CSSLengthIsValid(length) && (length.units == UnitsPct))
            result.textIndentPct = length.value;
    }

    result.totalPct = result.marginLeftPct + result.textIndentPct;

    return result;
}


static int listProperties(WordConverter *conv, const char *numId, const char *ilvl, CSSProperties *properties)
{
    WordConcreteNum *num = WordNumberingConcreteWithId(conv->numbering,numId);
    if (num == NULL)
        return 0;;
    WordNumLevel *level = WordConcreteNumGetLevel(num,atoi(ilvl));
    if (level == NULL)
        return 0;;
    DFNode *pPr = DFChildWithTag(level->element,WORD_PPR);
    if (pPr == NULL)
        return 0;;
    const char *styleId = NULL;
    WordGetPPr(pPr,properties,&styleId,conv->mainSection);
    return 1;
}

static ListDimensions listIndent(WordConverter *conv, const char *numId, const char *ilvl)
{
    CSSProperties *properties = CSSPropertiesNew();
    listProperties(conv,numId,ilvl,properties);
    ListDimensions result = cssPropertiesIndent(properties);
    CSSPropertiesRelease(properties);
    return result;
}

// FIXME: make private to this file
double listDesiredIndent(WordConverter *conv, const char *numId, const char *ilvl)
{
    CSSProperties *properties = CSSPropertiesNew();
    listProperties(conv,numId,ilvl,properties);
    double result = 0.0;

    if (CSSGet(properties,"margin-left") != NULL) {
        CSSLength length = CSSLengthFromString(CSSGet(properties,"margin-left"));
        if (CSSLengthIsValid(length) && (length.units == UnitsPct))
            result += length.value;
    }

    CSSPropertiesRelease(properties);
    return result;
}

static ListDimensions paragraphIndent(DFNode *p)
{
    if (p->tag < MIN_ELEMENT_TAG)
        return ListDimensionsZero;;
    const char *cssText = DFGetAttribute(p,HTML_STYLE);
    CSSProperties *properties = CSSPropertiesNewWithString(cssText);
    ListDimensions result = cssPropertiesIndent(properties);
    CSSPropertiesRelease(properties);
    return result;
}

static void adjustMarginLeft(DFNode *element, double adjustPct, int noTextIndent)
{
    if ((element->tag != HTML_TABLE) && !HTML_isParagraphTag(element->tag))
        return;;

    const char *cssText = DFGetAttribute(element,HTML_STYLE);
    CSSProperties *properties = CSSPropertiesNewWithString(cssText);

    double oldMarginLeft = 0;
    if (CSSGet(properties,"margin-left") != NULL) {
        CSSLength length = CSSLengthFromString(CSSGet(properties,"margin-left"));
        if (CSSLengthIsValid(length) && (length.units == UnitsPct))
            oldMarginLeft = length.value;

        if (CSSGet(properties,"width") != NULL) {
            CSSLength length = CSSLengthFromString(CSSGet(properties,"width"));
            if (CSSLengthIsValid(length) && (length.units == UnitsPct)) {
                double oldWidth = length.value;
                double newWidth = oldWidth + oldMarginLeft;
                char buf[100];
                CSSPut(properties,"width",DFFormatDoublePct(buf,100,newWidth));
            }
        }
    }

    double oldTextIndent = 0;
    if (CSSGet(properties,"text-indent") != NULL) {
        CSSLength length = CSSLengthFromString(CSSGet(properties,"text-indent"));
        if (CSSLengthIsValid(length) && (length.units == UnitsPct))
            oldTextIndent = length.value;
    }

    double newMarginLeft = oldMarginLeft + adjustPct;
    double newTextIndent = oldTextIndent;

    if (newMarginLeft < 0)
        newMarginLeft = 0;
    if (fabs(newMarginLeft) >= 0.01) {
        char buf[100];
        CSSPut(properties,"margin-left",DFFormatDoublePct(buf,100,newMarginLeft));
    }
    else {
        CSSPut(properties,"margin-left",NULL);
    }

    if (noTextIndent) {
        CSSPut(properties,"text-indent",NULL);
    }
    else if (newTextIndent < -newMarginLeft) {
        // Don't allow negative text-indent
        newTextIndent = -newMarginLeft;
        if (fabs(newTextIndent) >= 0.01) {
            char buf[100];
            CSSPut(properties,"text-indent",DFFormatDoublePct(buf,100,newTextIndent));
        }
        else {
            CSSPut(properties,"text-indent",NULL);
        }
    }

    char *propertiesText = CSSPropertiesCopyDescription(properties);
    if (strlen(propertiesText) == 0)
        DFRemoveAttribute(element,HTML_STYLE);
    else
        DFSetAttribute(element,HTML_STYLE,propertiesText);
    free(propertiesText);

    CSSPropertiesRelease(properties);
}

static int isEmptyParagraph(DFNode *p)
{
    if (p->tag != HTML_P)
        return 0;
    for (DFNode *child = p->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case HTML_BR:
                break;
            default:
                return 0;
        }
    }
    return 1;
}

static void fixTrailingParagraphs(ListStack *stack, int minIlvl)
{
    if (stack->top == NULL)
        return;;

    DFNode *li = stack->top->element->last;
    if (li == NULL)
        return;;

    DFNode *child = li->first;

    // Move to first element
    while ((child != NULL) && (child->tag < MIN_ELEMENT_TAG))
        child = child->next;

    if (child != NULL) {
        // Adjust indentation of first element
        if ((stack->top != NULL) && (child->tag >= MIN_ELEMENT_TAG)) {
            double pct = stack->top->dimensions.marginLeftPct;
            adjustMarginLeft(child,-pct,1);
        }

        // Skip past first element
        child = child->next;
    }

    DFNode *preceding = (child != NULL) ? child->prev : NULL;
    DFNode *parent = li;
    while (child != NULL) {
        ListDimensions dimensions = paragraphIndent(child);

        if (!isEmptyParagraph(child) || (minIlvl < 0)) {
            while ((stack->top != NULL) &&
                   (stack->top->ilvl >= minIlvl) &&
                   (dimensions.totalPct < stack->top->dimensions.totalPct - 0.001)) {
                assert(stack->top != NULL);
                preceding = stack->top->element;
                parent = stack->top->element->parent;
                ListStackPop(stack);
            }
        }

        if ((stack->top != NULL) && (child->tag >= MIN_ELEMENT_TAG)) {
            double pct = stack->top->dimensions.marginLeftPct;
            adjustMarginLeft(child,-pct,0);
        }

        DFNode *next = child->next;
        DFInsertBefore(parent,child,preceding ? preceding->next : NULL);
        preceding = child;
        child = next;
    }
}

static void Word_fixListSingle(WordConverter *conv, DFNode *node)
{
    ListStack stack;
    bzero(&stack,sizeof(ListStack));

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;

        int isListItem = 0;

        if (child->tag == HTML_P) {
            DFNode *elem = child;

            const char *numIdStr = DFGetAttribute(elem,WORD_NUMID);
            const char *ilvlStr = DFGetAttribute(elem,WORD_ILVL);
            DFRemoveAttribute(elem,WORD_NUMID);
            DFRemoveAttribute(elem,WORD_ILVL);

            // A numId of 0 means that there is no numbering applied to this paragraph
            if ((numIdStr != NULL) && (atoi(numIdStr) == 0)) {
                numIdStr = NULL;
                ilvlStr = NULL;
            }

            if ((numIdStr != NULL) && (ilvlStr != NULL)) {
                isListItem = 1;
                int numId = atoi(numIdStr);
                int ilvl = atoi(ilvlStr);
                ListDimensions dimensions = listIndent(conv,numIdStr,ilvlStr);

                // Find the list at the same ilvl, and check if it has the same numId. If not, we're
                // starting a new list.

                ListFrame *sameLevelFrame = NULL;
                for (ListFrame *frame = stack.top; frame != NULL; frame = frame->parent) {
                    if (frame->ilvl == ilvl)
                        sameLevelFrame = frame;
                }

                if ((sameLevelFrame != NULL) && (sameLevelFrame->numId != numId))
                    fixTrailingParagraphs(&stack,ilvl);
                else
                    fixTrailingParagraphs(&stack,ilvl+1);

                if ((stack.top != NULL) && (stack.top->numId != numId))
                    ListStackPopToAboveIlvl(&stack,ilvl);
                else if ((stack.top != NULL) && (stack.top->ilvl > ilvl))
                    ListStackPopToAboveIlvl(&stack,ilvl+1);

                if ((stack.top == NULL) || (stack.top->numId != numId) || (stack.top->ilvl < ilvl)) {
                    WordConcreteNum *num = WordNumberingConcreteWithId(conv->numbering,numIdStr);
                    WordNumLevel *level = (num != NULL) ? WordConcreteNumGetLevel(num,ilvl) : NULL;

                    const char *type = WordNumLevelToListStyleType(level);
                    Tag tag;
                    if (DFStringEquals(type,"disc") ||
                        DFStringEquals(type,"circle") ||
                        DFStringEquals(type,"square"))
                        tag = HTML_UL;
                    else
                        tag = HTML_OL;

                    DFNode *element = DFCreateElement(conv->html,tag);

                    if (type != NULL)
                        DFFormatAttribute(element,HTML_STYLE,"list-style-type: %s",type);

                    if (stack.top != NULL) {
                        DFNode *li;
                        if (stack.top->element->last != NULL)
                            li = stack.top->element->last;
                        else
                            li = DFCreateChildElement(stack.top->element,HTML_LI);
                        DFAppendChild(li,element);
                    }
                    else {
                        DFInsertBefore(node,element,child);
                    }
                    ListStackPushFrame(&stack,element,numId,ilvl,dimensions);
                }
            }
        }

        if (stack.top != NULL) {
            DFNode *li;
            if ((stack.top->element->last != NULL) && !isListItem)
                li = stack.top->element->last;
            else
                li = DFCreateChildElement(stack.top->element,HTML_LI);
            DFAppendChild(li,child);
        }
    }
    fixTrailingParagraphs(&stack,-1);
    while (stack.top != NULL)
        ListStackPop(&stack);
}

static void Word_fixLists(WordConverter *conv, DFNode *node)
{
    int haveList = 0;
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if ((DFGetAttribute(child,WORD_NUMID) != NULL) &&
            (DFGetAttribute(child,WORD_ILVL) != NULL)) {
            haveList = 1;
            break;
        }
    }

    if (haveList)
        Word_fixListSingle(conv,node);;

    DFNode *next;

    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        Word_fixLists(conv,child);
    }
}

void WordPostProcessHTMLLists(WordConverter *conv)
{
    Word_fixLists(conv,conv->html->docNode);
}

static void Word_preProcessLists(WordConverter *word, DFNode *node, int ilvl);

static void Word_flattenList(WordConverter *word, DFNode *list, int ilvl, DFNode *parent, DFNode *nextSibling)
{
    const char *type = (list->tag == HTML_OL) ? "decimal" : "disc";
    const char *cssText = DFGetAttribute(list,HTML_STYLE);
    CSSProperties *properties = CSSPropertiesNewWithString(cssText);
    if (CSSGet(properties,"list-style-type") != NULL)
        type = CSSGet(properties,"list-style-type");

    DFFormatAttribute(list,CONV_LISTNUM,"%u",list->seqNo);
    DFFormatAttribute(list,CONV_ILVL,"%d",ilvl);
    DFSetAttribute(list,CONV_LISTTYPE,type);

    for (DFNode *li = list->first; li != NULL; li = li->next) {

        DFNode *first = li->first;
        DFNode *liChildNext;
        for (DFNode *liChild = li->first; liChild != NULL; liChild = liChildNext) {
            liChildNext = liChild->next;

            if ((liChild->tag == HTML_UL) || (liChild->tag == HTML_OL)) {
                Word_flattenList(word,liChild,ilvl+1,parent,nextSibling);
            }
            else {
                if (liChild->tag == HTML_P) {
                    DFFormatAttribute(liChild,CONV_LISTNUM,"%u",list->seqNo);
                    DFFormatAttribute(liChild,CONV_ILVL,"%d",ilvl);
                    DFSetAttribute(liChild,CONV_LISTTYPE,type);
                    if (liChild == first)
                        DFSetAttribute(liChild,CONV_LISTITEM,"true");
                }
                DFInsertBefore(parent,liChild,nextSibling);
                Word_preProcessLists(word,liChild,ilvl);
            }
        }
    }

    DFRemoveNode(list);
    CSSPropertiesRelease(properties);
}

static void Word_preProcessLists(WordConverter *word, DFNode *node, int ilvl)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;

        if ((child->tag == HTML_UL) || (child->tag == HTML_OL)) {
            Word_flattenList(word,child,ilvl+1,node,next);
        }
        else {
            Word_preProcessLists(word,child,ilvl);
        }
    }
}

void WordPreProcessHTMLLists(WordConverter *conv)
{
    Word_preProcessLists(conv,conv->html->docNode,-1);
}

static void fixWordLists(DFNode *node, WordConverter *conv)
{
    for (DFNode *child = node->first; child != NULL; child = child->next)
        fixWordLists(child,conv);

    int haveParagraphs = 0;
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if (child->tag == WORD_P) {
            haveParagraphs = 1;
            break;
        }
    }

    if (!haveParagraphs)
        return;

    int createdHashTables = 0;
    DFHashTable *replacementNumIds = NULL;
    DFHashTable *itemNoByListKey = NULL;
    DFHashTable *lastNumIdByIlvl = NULL;
    DFHashTable *itemNoByIlvl = NULL;
    int maxIlvl = -1;

    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if (child->tag != WORD_P)
            continue;
        DFNode *pPrElem = DFChildWithTag(child,WORD_PPR);
        DFNode *numPrElem = DFChildWithTag(pPrElem,WORD_NUMPR);
        DFNode *numIdElem = DFChildWithTag(numPrElem,WORD_NUMID);
        DFNode *ilvlElem = DFChildWithTag(numPrElem,WORD_ILVL);
        const char *numId = DFGetAttribute(numIdElem,WORD_VAL);
        const char *ilvl = DFGetAttribute(ilvlElem,WORD_VAL);

        if ((numId == NULL) || (atoi(numId) == 0))
            continue;

        if (!createdHashTables) {
            replacementNumIds = DFHashTableNew((DFCopyFunction)strdup,free);
            itemNoByListKey = DFHashTableNew((DFCopyFunction)strdup,free);
            lastNumIdByIlvl = DFHashTableNew((DFCopyFunction)strdup,free);
            itemNoByIlvl = DFHashTableNew((DFCopyFunction)strdup,free);
            createdHashTables = 1;
        }

        if (ilvl == NULL)
            ilvl = "0";;

        WordConcreteNum *concreteNum = WordNumberingConcreteWithId(conv->numbering,numId);
        // FIXME: Crash here if concreteNum is NULL
        WordNumLevel *numLevel = WordConcreteNumGetLevel(concreteNum,atoi(ilvl));

        const char *levelStart = NULL;

        if (numLevel != NULL) {
            for (DFNode *lvlChild = numLevel->element->first; lvlChild != NULL; lvlChild = lvlChild->next) {
                switch (lvlChild->tag) {
                    case WORD_START:
                        levelStart = DFGetAttribute(lvlChild,WORD_VAL);
                        break;
                }
            }
        }

        char *listKey = DFFormatString("%s:%s",numId,ilvl);
        char *itemNo = DFStrDup(DFHashTableLookup(itemNoByListKey,listKey));
        if (itemNo == NULL) {
            itemNo = strdup("1");

            if ((levelStart != NULL) && (atoi(levelStart) > 1) && (atoi(ilvl) <= maxIlvl)) {
                const char *prevNumId = DFHashTableLookup(lastNumIdByIlvl,ilvl);
                const char *prevItemNo = DFHashTableLookup(itemNoByIlvl,ilvl);

                if ((prevNumId != NULL) && (prevItemNo != NULL)) {
                    DFHashTableAdd(replacementNumIds,numId,prevNumId);
                    free(itemNo);
                    itemNo = DFFormatString("%d",atoi(prevItemNo)+1);
                }
            }
        }
        else {
            int intValue = atoi(itemNo);
            free(itemNo);
            itemNo = DFFormatString("%d",intValue+1);
        }
        DFHashTableAdd(itemNoByListKey,listKey,itemNo);

        const char *replNumId = DFHashTableLookup(replacementNumIds,numId);
        if (replNumId != NULL) {
            numId = replNumId;
            DFSetAttribute(numIdElem,WORD_VAL,numId);
        }

        DFHashTableAdd(lastNumIdByIlvl,ilvl,numId);
        DFHashTableAdd(itemNoByIlvl,ilvl,itemNo);
        maxIlvl = atoi(ilvl);
        free(listKey);
        free(itemNo);
    }
    DFHashTableRelease(replacementNumIds);
    DFHashTableRelease(itemNoByListKey);
    DFHashTableRelease(lastNumIdByIlvl);
    DFHashTableRelease(itemNoByIlvl);
}

void WordFixLists(WordConverter *conv)
{
    fixWordLists(conv->package->document->docNode,conv);
}
