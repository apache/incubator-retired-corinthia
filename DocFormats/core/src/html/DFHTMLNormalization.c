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
#include "DFHTMLNormalization.h"
#include "DFDOM.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "DFHTML.h"
#include "DFClassNames.h"
#include "DFString.h"
#include "DFCharacterSet.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void removeWhitespaceTextChildren(DFNode *node)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        if (DFIsWhitespaceNode(child))
            DFRemoveNode(child);
    }
}

static void wrapNodes(DFNode *first, DFNode *last, Tag tag)
{
    DFNode *wrapper = DFCreateElement(last->doc,tag);
    DFInsertBefore(last->parent,wrapper,last->next);
    DFNode *next;
    for (DFNode *child = first; 1; child = next) {
        next = child->next;
        DFAppendChild(wrapper,child);
        if (child == last)
            break;
    }
}

static void wrapAnonymousChildParagraphs(DFNode *node)
{
    // All children must be container or paragraph nodes
    DFNode *child = node->first;
    DFNode *anonFirst = NULL;
    DFNode *anonLast = NULL;
    int anonOnlyWhitespace = 1;
    while (1) {
        DFNode *next = child ? child->next : NULL;

        if ((child == NULL) || HTML_isBlockLevelTag(child->tag)) {
            if ((anonFirst != NULL) && (anonLast != NULL)) {

                if (!anonOnlyWhitespace)
                    wrapNodes(anonFirst,anonLast,HTML_P);

                anonFirst = NULL;
                anonLast = NULL;
                anonOnlyWhitespace = 1;
            }
        }

        if (child == NULL)
            break;

        if ((child != NULL) && !HTML_isBlockLevelTag(child->tag)) {
            anonFirst = (anonFirst != NULL) ? anonFirst : child;
            anonLast = child;
            if (anonOnlyWhitespace && !DFIsWhitespaceNode(child))
                anonOnlyWhitespace = 0;
        }

        child = next;
    }
}

static void mergeAdjacentTextNodes(DFNode *node)
{
    DFNode *child = node->first;
    while (child != NULL) {
        if ((child->tag == DOM_TEXT) && (child->next != NULL) && (child->next->tag == DOM_TEXT)) {
            // FIXME: no tests cover this case
            const char *value = child->value;
            const char *nextValue = child->next->value;
            char *mergedValue = DFFormatString("%s%s",value,nextValue);
            DFSetNodeValue(child,mergedValue);
            free(mergedValue);
            DFRemoveNode(child->next);
        }
        else {
            child = child->next;
        }
    }

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        mergeAdjacentTextNodes(child);
    }
}

typedef struct LeafEntry {
    DFNode *node;
    int depth;
    int spaceAtStart;
    int spaceAtEnd;
} LeafEntry;

static void findLeafNodes(DFNode *node, int depth, DFArray *leafEntries)
{
    if (node->first == NULL) {
        LeafEntry *entry = (LeafEntry *)calloc(1,sizeof(LeafEntry));
        entry->node = node;
        entry->depth = depth;
        DFArrayAppend(leafEntries,entry);
    }
    else {
        for (DFNode *child = node->first; child != NULL; child = child->next)
            findLeafNodes(child,depth+1,leafEntries);
    }
}

static void fixParagraphWhitespace(DFNode *paragraph)
{
    DFArray *leafEntries = DFArrayNew(NULL,free);
    findLeafNodes(paragraph,0,leafEntries);
    for (size_t i = 0; i < DFArrayCount(leafEntries); i++) {
        LeafEntry *entry = DFArrayItemAt(leafEntries,i);
        if (entry->node->tag != DOM_TEXT)
            continue;


//        char *quoted = DFQuote(entry->node->value);
//        printf("fixParagraphWhitespace: value = %s\n",quoted);
//        free(quoted);


        uint32_t *oldChars = DFUTF8To32(entry->node->value);
        size_t oldLen = DFUTF32Length(oldChars);
        uint32_t *newChars = (uint32_t *)malloc((oldLen+1)*sizeof(uint32_t));
        size_t newLen = 0;
        int haveSpace = 0;

        for (size_t pos = 0; pos < oldLen; pos++) {
            if (DFCharIsWhitespaceOrNewline(oldChars[pos]) && (oldChars[pos] != DFNbspChar)) {
                if (!haveSpace)
                    newChars[newLen++] = oldChars[pos];
                haveSpace = 1;
            }
            else {
                newChars[newLen++] = oldChars[pos];
                haveSpace = 0;
            }
        }
        newChars[newLen] = 0;

        char *newValue = DFUTF32to8(newChars);
        DFSetNodeValue(entry->node,newValue);
        free(newValue);

        if ((newLen > 0) && DFCharIsWhitespaceOrNewline(newChars[0]) && (newChars[0] != DFNbspChar))
            entry->spaceAtStart = 1;
        if ((newLen > 0) && DFCharIsWhitespaceOrNewline(newChars[newLen-1]) && (newChars[newLen-1] == DFNbspChar))
            entry->spaceAtEnd = 1;

        free(oldChars);
        free(newChars);
    }

    for (size_t i = 0; i < DFArrayCount(leafEntries); i++) {
        LeafEntry *entry = DFArrayItemAt(leafEntries,i);
        LeafEntry *prev = (i > 0) ? DFArrayItemAt(leafEntries,i-1) : NULL;
        LeafEntry *next = (i+1 < DFArrayCount(leafEntries)) ? DFArrayItemAt(leafEntries,i+1) : NULL;

        if (entry->node->tag != DOM_TEXT)
            continue;
        uint32_t *valuestart = DFUTF8To32(entry->node->value);
        uint32_t *valueptr = valuestart;
        size_t len = DFUTF32Length(valueptr);

        if ((i == 0) ||
            ((prev != NULL) && (prev->node->tag == HTML_BR)) ||
            (entry->spaceAtStart && (prev != NULL) && prev->spaceAtEnd && (entry->depth >= prev->depth))) {
            // FIXME: no tests cover this case
            size_t start = 0;
            while ((start < len) && DFCharIsWhitespaceOrNewline(valueptr[start]) && (valueptr[start] != DFNbspChar))
                start++;
            valueptr = &valueptr[start];
            char *newNodeValue = DFUTF32to8(valueptr);
            DFSetNodeValue(entry->node,newNodeValue);
            free(newNodeValue);
        }

        len = DFUTF32Length(valueptr);
        if ((i == DFArrayCount(leafEntries)-1) ||
            ((next != NULL) && (next->node->tag == HTML_BR)) ||
            (entry->spaceAtEnd && (next != NULL) && next->spaceAtStart && (entry->depth > next->depth))) {
            size_t end = len;
            while ((end > 0) && DFCharIsWhitespaceOrNewline(valueptr[end-1]) && (valueptr[end-1] != DFNbspChar))
                end--;
            valueptr[end] = 0;
            char *newNodeValue = DFUTF32to8(valueptr);
            DFSetNodeValue(entry->node,newNodeValue);
            free(newNodeValue);
        }
        free(valuestart);
    }

    // Delete any tempty text nodes and their containers
    // FIXME: no tests cover this case
    for (size_t i = 0; i < DFArrayCount(leafEntries); i++) {
        LeafEntry *entry = DFArrayItemAt(leafEntries,i);
        DFNode *node = entry->node;
        if ((node->tag == DOM_TEXT) && (strlen(node->value) == 0))
            DFRemoveNode(node);
    }
    DFArrayRelease(leafEntries);
}

static void mergeWithPrev(DFNode *node)
{
    DFNode *prev = node->prev;
    while (node->first != NULL) {
        if ((prev->last != NULL) && (prev->last->tag == DOM_TEXT) && (node->first->tag == DOM_TEXT)) {
            DFNode *prevText = prev->last;
            DFNode *curText = node->first;
            const char *prevValue = prevText->value;
            const char *curValue = curText->value;
            char *mergedValue = DFFormatString("%s%s",prevValue,curValue);
            DFSetNodeValue(prevText,mergedValue);
            free(mergedValue);
            DFRemoveNode(node->first);
        }
        else {
            DFAppendChild(prev,node->first);
        }
    }

    DFRemoveNode(node);
}

static void mergeWithNext(DFNode *node)
{
    DFNode *next = node->next;
    while (node->last != NULL) {
        if ((next->first != NULL) && (next->first->tag == DOM_TEXT) && (node->last->tag == DOM_TEXT)) {
            DFNode *curText = node->last;
            DFNode *nextText = next->first;
            const char *curValue = curText->value;
            const char *nextValue = nextText->value;
            char *mergedValue = DFFormatString("%s%s",curValue,nextValue);
            DFSetNodeValue(nextText,mergedValue);
            free(mergedValue);
            DFRemoveNode(node->last);
        }
        else {
            DFInsertBefore(next,node->last,next->first);
        }
    }
    DFRemoveNode(node);
}

static int canMergeText(DFNode *a, DFNode *b)
{
    return ((a->last != NULL) && (a->last->tag == DOM_TEXT) &&
            (b->first != NULL) && (b->first->tag == DOM_TEXT));
}

static int containsImage(DFNode *node)
{
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if (child->tag == HTML_IMG)
            return 1;
    }
    return 0;
}

static void mergeSpans(DFNode *node)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;

        if (child->tag != HTML_SPAN)
            continue;

        if (DFGetAttribute(child,HTML_ID) != NULL)
            continue;

        if (containsImage(child))
            continue;

        DFNode *before = NULL;
        DFNode *after = NULL;

        if ((child->prev != NULL) &&
            (child->prev->tag == HTML_SPAN) &&
            !HTML_isSpecialSpan(child->prev) &&
            identicalAttributesExcept(child->prev,child,HTML_ID)) {
            before = child->prev;
        }

        if ((child->next != NULL) &&
            (child->next->tag == HTML_SPAN) &&
            !HTML_isSpecialSpan(child->next) &&
            identicalAttributesExcept(child->next,child,HTML_ID)) {
            after = child->next;
        }

        if ((before != NULL) && containsImage(before))
            continue;

        if ((after != NULL) && containsImage(after))
            continue;

        if ((before != NULL) && canMergeText(before,child))
            mergeWithPrev(child);
        else if ((after != NULL) && canMergeText(child,after))
            mergeWithNext(child);
        else if (before != NULL)
            mergeWithPrev(child);
        else if (after != NULL)
            mergeWithNext(child);
    }
}

static void addLeaf(DFNode *node, DFNode *dest, CSSProperties *properties, char **spanId, const char *className)
{
    DFNode *span = DFCreateElement(dest->doc,HTML_SPAN);

    if (!CSSPropertiesIsEmpty(properties)) {
        char *propertiesText = CSSPropertiesCopyDescription(properties);
        DFSetAttribute(span,HTML_STYLE,propertiesText);
        free(propertiesText);
    }

    if (className != NULL)
        DFSetAttribute(span,HTML_CLASS,className);

    if (*spanId != NULL) {
        if (!DFStringEquals(*spanId,DFGetAttribute(node,HTML_ID)))
            DFSetAttribute(span,HTML_ID,(*spanId));
        free(*spanId);
        *spanId = NULL;
    }

    if (node != NULL)
        DFAppendChild(span,node);
    DFAppendChild(dest,span);
}

static void normalizeInline(DFNode *source, DFNode *dest, CSSProperties *properties, int depth, char **spanId,
                            const char *className)
{
    if (source == dest) {
        source = DFCreateElement(dest->doc,dest->tag);
        while (dest->first != NULL)
            DFAppendChild(source,dest->first);
    }

    properties = CSSPropertiesRetain(properties);
    DFNode *next;
    for (DFNode *node = source->first; node != NULL; node = next) {
        next = node->next;

        const char *oldClassName = className;
        CSSProperties *oldProperties = properties;

        const char *nodeClass = DFGetAttribute(node,HTML_CLASS);
        int container = DFStringEquals(nodeClass,DFContainerClass);
        int placeholder = DFStringEquals(nodeClass,DFPlaceholderClass);
        if ((nodeClass != NULL) && !container && !placeholder)
            className = nodeClass;;

        const char *nodeStyle = DFGetAttribute(node,HTML_STYLE);
        if (nodeStyle != NULL) {
            CSSProperties *replaced = properties;
            properties = CSSPropertiesNewWithExtra(replaced,nodeStyle);
            CSSPropertiesRelease(replaced);
        }

        switch (node->tag) {
            case HTML_B: {
                int oldBold = CSSGetBold(properties);
                CSSSetBold(properties,1);
                normalizeInline(node,dest,properties,depth+1,spanId,className);
                CSSSetBold(properties,oldBold);
                break;
            }
            case HTML_I: {
                int oldItalic = CSSGetItalic(properties);
                CSSSetItalic(properties,1);
                normalizeInline(node,dest,properties,depth+1,spanId,className);
                CSSSetItalic(properties,oldItalic);
                break;
            }
            case HTML_U: {
                int oldUnderline = CSSGetUnderline(properties);
                CSSSetUnderline(properties,1);
                normalizeInline(node,dest,properties,depth+1,spanId,className);
                CSSSetUnderline(properties,oldUnderline);
                break;
            }
            case HTML_SPAN: {
                if (DFStringEquals(nodeClass,"footnote") || DFStringEquals(nodeClass,"endnote")) {
                    normalizeInline(node,node,properties,0,spanId,NULL);
                    DFAppendChild(dest,node);
                    break;
                }

                if ((nodeClass != NULL) && DFStringEquals(nodeClass,DFPlaceholderClass)) {
                    addLeaf(node,dest,properties,spanId,className);
                    break;
                }
                const char *thisId = DFGetAttribute(node,HTML_ID);
                if ((depth == 0) && !container) {
                    free(*spanId);
                    *spanId = DFStrDup(thisId);
                }
                if (DFStringHasPrefix(nodeClass,"uxwrite-") && (container || (node->first == NULL))) {
                    normalizeInline(node,node,properties,depth+1,spanId,className);

                    if (!CSSPropertiesIsEmpty(properties) || (*spanId != NULL))
                        addLeaf(node,dest,properties,spanId,NULL);
                    else
                        DFAppendChild(dest,node);
                }
                else {
                    normalizeInline(node,dest,properties,depth+1,spanId,className);
                }

                // Even if the span is empty, the run that it corresponds to may contain an
                // unsupported element, so we need to keep the span to avoid losing said element
                // on update.
                if ((*spanId != NULL) && DFStringEquals(*spanId,thisId))
                    addLeaf(NULL,dest,properties,spanId,className);

                break;
            }
            case HTML_INS:
            case HTML_DEL:
            case HTML_A: {
                normalizeInline(node,node,properties,depth+1,spanId,className);
                DFAppendChild(dest,node);
                break;
            }
            case DOM_TEXT:
            case HTML_IMG: {
                addLeaf(node,dest,properties,spanId,className);
                break;
            }
            case HTML_BR:
                // <br> elements that are the only child of their containing paragraph are special,
                // in that they are used to signify a paragarph which has no content (this is
                // required for them to display as visible space in browsers). However they do
                // *not* constitute additional line breaks that should be included in word documents.
                if ((node->parent != NULL) && HTML_isParagraphTag(node->parent->tag) && (node->next == NULL))
                    DFAppendChild(dest,node);
                else
                    addLeaf(node,dest,properties,spanId,className);
                break;
            default: {
                normalizeInline(node,dest,properties,depth+1,spanId,className);
                DFAppendChild(dest,node);
                break;
            }
        }

        className = oldClassName;
        CSSProperties *replaced = properties;
        properties = CSSPropertiesRetain(oldProperties);
        CSSPropertiesRelease(replaced);
    }
    CSSPropertiesRelease(properties);
}

static void fixRunContentHierarchy(DFNode *node)
{
    if (node->tag == HTML_SPAN) {
        const char *className = DFGetAttribute(node,HTML_CLASS);
        if (DFStringEquals(className,DFTabClass)) {
            if (node->parent->tag != HTML_SPAN) {
                DFNode *wrapper = DFCreateElement(node->doc,HTML_SPAN);
                DFInsertBefore(node->parent,wrapper,node);
                DFAppendChild(wrapper,node);
            }
        }
    }

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        fixRunContentHierarchy(child);
    }
}

static void normalizeParagraph(DFNode *paragraph)
{
    DFNode *next;

    for (DFNode *child = paragraph->first; child != NULL; child = next) {
        next = child->next;

        if (DFIsWhitespaceNode(child)) {
            DFNode *span = DFCreateElement(paragraph->doc,HTML_SPAN);
            DFInsertBefore(paragraph,span,child);
            DFAppendChild(span,child);
        }
    }

    fixParagraphWhitespace(paragraph);

    // FIXME: Properly handle images, links, and other non-text inline elements

    char *spanId = NULL;
    CSSProperties *empty = CSSPropertiesNew();
    normalizeInline(paragraph,paragraph,empty,0,&spanId,NULL);
    CSSPropertiesRelease(empty);
    fixRunContentHierarchy(paragraph);
    free(spanId);

    mergeSpans(paragraph);
}

static void normalizeContainer(DFNode *container);

static void normalizeUnknownContainer(DFNode *child)
{
    wrapAnonymousChildParagraphs(child);
    removeWhitespaceTextChildren(child);
    normalizeContainer(child);
    DFRemoveNodeButKeepChildren(child);
}

static void normalizeContainer(DFNode *container)
{
    DFNode *next;
    for (DFNode *child = container->first; child != NULL; child = next) {
        next = child->next;
        switch (child->tag) {
            case HTML_H1:
            case HTML_H2:
            case HTML_H3:
            case HTML_H4:
            case HTML_H5:
            case HTML_H6:
            case HTML_P:
            case HTML_CAPTION:
            case HTML_FIGCAPTION:
                normalizeParagraph(child);
                break;
            case HTML_BODY:
            case HTML_TD:
            case HTML_TH:
            case HTML_LI:
            case HTML_FIGURE:
                // All children must be a paragraph, heading, list, or table
                wrapAnonymousChildParagraphs(child);
                removeWhitespaceTextChildren(child);
                normalizeContainer(child);
                break;
            case HTML_TABLE:
            case HTML_THEAD:
            case HTML_TBODY:
            case HTML_TFOOT:
            case HTML_TR:
            case HTML_UL:
            case HTML_OL:
                removeWhitespaceTextChildren(child);
                normalizeContainer(child);
                break;
            case HTML_HEAD:
                break;
            case HTML_NAV: {
                const char *className = DFGetAttribute(child,HTML_CLASS);
                if (DFStringEquals(className,DFTableOfContentsClass) ||
                    DFStringEquals(className,DFListOfFiguresClass) ||
                    DFStringEquals(className,DFListOfTablesClass)) {
                    normalizeContainer(child);
                }
                else {
                    normalizeUnknownContainer(child);
                }
                break;
            }
            default:
                normalizeUnknownContainer(child);
                break;
        }
    }
}

void HTML_normalizeDocument(DFDocument *doc)
{
    assert(doc->root != NULL);
    mergeAdjacentTextNodes(doc->root);
    normalizeContainer(doc->root);
}

static DFHashTable *extractInlineProperties(DFNode *paragraph)
{
    DFHashTable *inlineProperties = DFHashTableNew((DFCopyFunction)strdup,free);
    const char *paraCSSText = DFGetAttribute(paragraph,HTML_STYLE);
    CSSProperties *paraProperties = CSSPropertiesNewWithString(paraCSSText);
    const char **allNames = CSSPropertiesCopyNames(paraProperties);
    for (int i = 0; allNames[i]; i++) {
        const char *name = allNames[i];
        if (CSSIsInlineProperty(name)) {
            const char *value = CSSGet(paraProperties,name);
            DFHashTableAdd(inlineProperties,name,value);
            CSSPut(paraProperties,name,NULL);
        }
    }
    free(allNames);
    char *propertiesText = CSSPropertiesCopyDescription(paraProperties);
    if (strlen(propertiesText) == 0)
        DFRemoveAttribute(paragraph,HTML_STYLE);
    else
        DFSetAttribute(paragraph,HTML_STYLE,propertiesText);
    free(propertiesText);
    CSSPropertiesRelease(paraProperties);
    return inlineProperties;
}

void HTML_pushDownInlineProperties(DFNode *node)
{
    if (HTML_isParagraphTag(node->tag)) {
        DFHashTable *inlineProperties = extractInlineProperties(node);
        if (DFHashTableCount(inlineProperties) == 0) {
            DFHashTableRelease(inlineProperties);
            return;
        }
        for (DFNode *child = node->first; child != NULL; child = child->next) {
            if (child->tag != HTML_SPAN)
                continue;
            const char *cssText = DFGetAttribute(child,HTML_STYLE);
            CSSProperties *properties = CSSPropertiesNewWithString(cssText);
            const char **allNames = DFHashTableCopyKeys(inlineProperties);
            for (int i = 0; allNames[i]; i++) {
                const char *name = allNames[i];
                const char *value = DFHashTableLookup(inlineProperties,name);
                CSSPut(properties,name,value);
            }
            free(allNames);
            char *propertiesText = CSSPropertiesCopyDescription(properties);
            if (strlen(propertiesText) == 0)
                DFRemoveAttribute(child,HTML_STYLE);
            else
                DFSetAttribute(child,HTML_STYLE,propertiesText);
            free(propertiesText);
            CSSPropertiesRelease(properties);
        }
        DFHashTableRelease(inlineProperties);
    }
    else {
        for (DFNode *child = node->first; child != NULL; child = child->next)
            HTML_pushDownInlineProperties(child);
    }
}
