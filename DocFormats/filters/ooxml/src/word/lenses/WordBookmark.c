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

#include "WordBookmark.h"
#include "WordLenses.h"
#include "DFDOM.h"
#include "WordPackage.h"
#include "WordField.h"
#include "WordObjects.h"
#include "WordCaption.h"
#include "WordSheet.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void findTargetAndType(WordBookmark *bookmark, WordSheet *sheet);
void findLabel(WordBookmark *bookmark);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordBookmark                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

const char *WordBookmarkTypeString(WordBookmarkType type)
{
    switch (type) {
        case WordBookmarkCursor:
            return "Cursor";
        case WordBookmarkHeading:
            return "Heading";
        case WordBookmarkTable:
            return "Table";
        case WordBookmarkFigure:
            return "Figure";
        case WordBookmarkEquation:
            return "Equation";
        default:
            return "Unknown";
    }
}

int WordBookmarkTypeHasLabel(WordBookmarkType type)
{
    switch (type) {
        case WordBookmarkHeading:
        case WordBookmarkTable:
        case WordBookmarkFigure:
            return 1;
        default:
            return 0;
    }
}

WordBookmark *WordBookmarkNew(const char *bookmarkId1, const char *bookmarkName1)
{
    WordBookmark *bookmark = (WordBookmark *)calloc(1,sizeof(WordBookmark));
    bookmark->retainCount = 1;
    bookmark->bookmarkId = (bookmarkId1 != NULL) ? strdup(bookmarkId1) : NULL;
    bookmark->bookmarkName = (bookmarkName1 != NULL) ? strdup(bookmarkName1) : NULL;
    bookmark->type = WordBookmarkUnknown;
    return bookmark;
}

WordBookmark *WordBookmarkRetain(WordBookmark *bookmark)
{
    if (bookmark != NULL)
        bookmark->retainCount++;
    return bookmark;
}

void WordBookmarkRelease(WordBookmark *bookmark)
{
    if ((bookmark == NULL) || (--bookmark->retainCount > 0))
        return;

    free(bookmark->bookmarkId);
    free(bookmark->bookmarkName);
    free(bookmark->label);
    free(bookmark);
}

void WordBookmarkAnalyze(WordBookmark *bookmark, WordSheet *sheet)
{
    // FIXME: Check if the following line is still relevant with the new bookmarks model
    if (bookmark->element == NULL) // new bookmark
        return;
    findTargetAndType(bookmark,sheet);
    findLabel(bookmark);
}

static void findAllNodes(DFNode *node, DFArray *result)
{
    if (node->tag == WORD_PPR)
        return;
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        DFArrayAppend(result,child);
        findAllNodes(child,result);
    }
}

CaptionParts WordBookmarkGetCaptionParts(WordBookmark *bookmark)
{
    CaptionParts parts;
    parts.beforeNum = 0;
    parts.num = 0;
    parts.afterNum = 0;

    // FIXME: Check if the following line is still relevant with the new bookmarks model
    if (bookmark->element == NULL)
        return parts;;

    DFArray *nodes = DFArrayNew(NULL,NULL);
    findAllNodes(bookmark->element,nodes);

    for (size_t i = 0; i < DFArrayCount(nodes); i++) {
        DFNode *node = DFArrayItemAt(nodes,i);
        if (node->tag == WORD_FLDSIMPLE) {
            const char *instr = DFGetAttribute(node,WORD_INSTR);
            if (instr != NULL) {
                const char **args = Word_parseField(instr);
                if ((args[0] != NULL) && !strcmp(args[0],"SEQ"))
                    parts.num = 1;
                free(args);
            }
        }
        else if (node->tag != WORD_BOOKMARK) {
            if (!parts.num)
                parts.beforeNum = 1;
            else
                parts.afterNum = 1;
        }
    }
    DFArrayRelease(nodes);

    return parts;
}

void findLabel(WordBookmark *bookmark)
{
    // FIXME: Not covered by tests
    DFBuffer *buffer = DFBufferNew();
    for (DFNode *child = bookmark->element->first; child != NULL; child = child->next) {
        // FIXME: handle inserted and deleted text
        if (child->tag == WORD_R)
            DFNodeTextToBuffer(child,buffer);
    }
    free(bookmark->label);
    bookmark->label = strdup(buffer->data);
    DFBufferRelease(buffer);
}

static int isHeadingOutlineLvl(const char *outlineLvl)
{
    if ((outlineLvl == NULL) || (strlen(outlineLvl) == 0))
        return 0;
    else
        return (atoi(outlineLvl) != 9);
}

static DFNode *findPreviousElement(DFNode *node)
{
    node = node->prev;
    while ((node != NULL) && (node->tag < MIN_ELEMENT_TAG))
        node = node->prev;
    return node;
}

static void findTargetAndType(WordBookmark *bookmark, WordSheet *sheet)
{
    if (DFStringEquals(bookmark->bookmarkName,"_GoBack")) {
        bookmark->type = WordBookmarkCursor;
        bookmark->target = NULL;
        return;
    }
    // FIXME: Check if the following line is still relevant with the new bookmarks model
    assert(bookmark->element != NULL);
    DFNode *p = WordFindContainingParagraph(bookmark->element);
    if (p == NULL)
        return;;
    DFNode *pPr = DFChildWithTag(p,WORD_PPR);
    if (pPr == NULL)
        return;;
    DFNode *pStyle = DFChildWithTag(pPr,WORD_PSTYLE);
    if (pStyle == NULL)
        return;;
    const char *styleId = DFGetAttribute(pStyle,WORD_VAL);
    if (styleId == NULL)
        return;;
    WordStyle *style = WordSheetStyleForTypeId(sheet,"paragraph",styleId);
    if ((style != NULL) && isHeadingOutlineLvl(style->outlineLvl)) {
        bookmark->type = WordBookmarkHeading;
        bookmark->target = p;
    }
    else if (DFStringEquals(styleId,"Caption")) {
        DFNode *prev = findPreviousElement(p);
        if (prev == NULL)
            return;

        if (prev->tag == WORD_TBL) {
            bookmark->type = WordBookmarkTable;
            bookmark->target = prev;
        }
        else if (Word_isFigureParagraph(prev)) {
            bookmark->type = WordBookmarkFigure;
            bookmark->target = prev;
        }
        else if (Word_isEquationParagraph(prev)) {
            bookmark->type = WordBookmarkEquation;
            bookmark->target = prev;
        }
    }
}

DFNode *WordFindContainingParagraph(DFNode *node)
{
    while ((node != NULL) && (node->tag != WORD_P))
        node = node->parent;
    return node;
}

// References: String (id attribute) -> DFArray of A Element objects

static void findReferencesRecursive(DFNode *node, DFHashTable *referencesById)
{
    if (node->tag == HTML_A) {
        const char *href = DFGetAttribute(node,HTML_HREF);
        if ((href != NULL) && (href[0] == '#')) {
            const char *targetId = &href[1];
            DFArray *links = DFHashTableLookup(referencesById,targetId);
            if (links == NULL) {
                links = DFArrayNew(NULL,NULL);
                DFHashTableAdd(referencesById,targetId,links);
                DFArrayRelease(links);
            }
            DFArrayAppend(links,node);
        }
    }
    for (DFNode *child = node->first; child != NULL; child = child->next)
        findReferencesRecursive(child,referencesById);
}

static DFHashTable *findReferences(DFDocument *doc)
{
    DFHashTable *references = DFHashTableNew((DFCopyFunction)DFArrayRetain,(DFFreeFunction)DFArrayRelease);
    findReferencesRecursive(doc->docNode,references);
    return references;
}

static WordBookmark *createBookmark(WordPutData *put)
{
    WordBookmark *bookmark = WordObjectsAddBookmark(put->conv->objects);
    DFNode *bookmarkSpan = DFCreateElement(put->conv->html,HTML_SPAN);
    DFSetAttribute(bookmarkSpan,HTML_CLASS,DFBookmarkClass);
    DFSetAttribute(bookmarkSpan,WORD_NAME,bookmark->bookmarkName);
    DFSetAttribute(bookmarkSpan,WORD_ID,bookmark->bookmarkId);
    bookmark->element = bookmarkSpan;
    return bookmark;
}

void Word_setupBookmarkLinks(WordPutData *put)
{
    DFHashTable *referencesById = findReferences(put->conv->html);
    const char **sortedIds = DFHashTableCopyKeys(referencesById);
    DFSortStringsCaseSensitive(sortedIds);
    for (int idIndex = 0; sortedIds[idIndex]; idIndex++) {
        const char *targetId = sortedIds[idIndex];
        DFArray *references = DFHashTableLookup(referencesById,targetId);
        DFNode *targetElem = DFElementForIdAttr(put->conv->html,targetId);
        if (targetElem == NULL)
            continue;

        // The following is only relevant for figures and tables
        int refText = 0;
        int refLabelNum = 0;
        int refCaptionText = 0;

        for (size_t refIndex = 0; refIndex < DFArrayCount(references); refIndex++) {
            DFNode *a = DFArrayItemAt(references,refIndex);
            const char *className = DFGetAttribute(a,HTML_CLASS);
            if (DFStringEquals(className,DFRefTextClass))
                refText = 1;
            else if (DFStringEquals(className,DFRefLabelNumClass))
                refLabelNum = 1;
            else if (DFStringEquals(className,DFRefCaptionTextClass))
                refCaptionText = 1;
        }

        DFNode *concrete = WordConverterGetConcrete(put,targetElem);
        switch (targetElem->tag) {
            case HTML_H1:
            case HTML_H2:
            case HTML_H3:
            case HTML_H4:
            case HTML_H5:
            case HTML_H6: {
                const char *bookmarkId = NULL;
                const char *bookmarkName = NULL;
                DFNode *bookmarkElem = NULL;
                if ((concrete != NULL) && (concrete->tag == WORD_P)) {

                    // FIXME: We only want to consider the bookmark to be the headings "correct"
                    // bookmark in the case where it contains all of the heading's content, though
                    // excluding other bookmarks that might come before or after it.

                    // If you have the cursor inside a heading bookmark when you save the document,
                    // word puts a bookmark called _GoBack there, and we of course don't want to
                    // confuse that with the actual heading's bookmark (if any).

                    // For now as a temporary hack we just explicitly filter out _GoBack; but there
                    // needs to be a more general fix, as there may be other bookmarks that end up
                    // in the heading.

                    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
                        if ((child->tag == WORD_BOOKMARK) &&
                            !DFStringEquals(DFGetAttribute(child,WORD_NAME),"_GoBack")) {
                            bookmarkElem = child;
                            bookmarkId = DFGetAttribute(bookmarkElem,WORD_ID);
                            bookmarkName = DFGetAttribute(bookmarkElem,WORD_NAME);
                            break;
                        }
                    }
                }

                if ((bookmarkElem == NULL) || (bookmarkId == NULL) || (bookmarkName == NULL)) {
                    // New bookmark
                    WordBookmark *bookmark = WordObjectsAddBookmark(put->conv->objects);
                    bookmarkId =bookmark->bookmarkId;
                    bookmarkName = bookmark->bookmarkName;
                }

                DFNode *bookmarkSpan = DFCreateElement(put->conv->html,HTML_SPAN);
                DFSetAttribute(bookmarkSpan,HTML_CLASS,DFBookmarkClass);

                if (bookmarkElem != NULL) {
                    // FIXME: Not covered by tests
                    DFFormatAttribute(bookmarkSpan,HTML_ID,"%s%u",put->conv->idPrefix,bookmarkElem->seqNo);
                }

                DFSetAttribute(bookmarkSpan,WORD_NAME,bookmarkName);
                DFSetAttribute(bookmarkSpan,WORD_ID,bookmarkId);

                while (targetElem->first != NULL)
                    DFAppendChild(bookmarkSpan,targetElem->first);
                DFAppendChild(targetElem,bookmarkSpan);

                break;
            }
            case HTML_TABLE:
            case HTML_FIGURE: {
                WordCaption *caption = WordObjectsCaptionForTarget(put->conv->objects,targetElem);
                if (caption == NULL)
                    break;

                assert(caption->element != NULL);
                assert((caption->number == NULL) || (caption->number->parent == caption->element));
                assert((caption->contentStart == NULL) || (caption->contentStart->parent == caption->element));

                // Note: caption.number may be null (i.e. if the caption is unnumbered)
                //       caption.contentStart may be null (if there is no text in the caption)

                WordBookmark *captionTextBookmark = NULL;
                WordBookmark *labelNumBookmark = NULL;
                WordBookmark *textBookmark = NULL;

                if (!refCaptionText && !refLabelNum && !refText)
                    refText = 1;

                if (refCaptionText) {
                    captionTextBookmark = createBookmark(put);
                    DFNode *nnext;
                    for (DFNode *n = caption->contentStart; n != NULL; n = nnext) {
                        nnext = n->next;
                        DFAppendChild(captionTextBookmark->element,n);
                    }
                    DFAppendChild(caption->element,captionTextBookmark->element);
                }
                if (refLabelNum && (caption->number != NULL)) {
                    labelNumBookmark = createBookmark(put);
                    DFNode *numberNext = caption->number->next;
                    DFNode *nnext;
                    for (DFNode *n = caption->element->first; (n != NULL) && (n != numberNext); n = nnext) {
                        nnext = n->next;
                        DFAppendChild(labelNumBookmark->element,n);
                    }
                    DFInsertBefore(caption->element,labelNumBookmark->element,caption->element->first);
                }
                if (refText) {
                    textBookmark = createBookmark(put);
                    DFNode *nnext;
                    for (DFNode *n = caption->element->first; n != NULL; n = nnext) {
                        nnext = n->next;
                        DFAppendChild(textBookmark->element,n);
                    }
                    DFAppendChild(caption->element,textBookmark->element);
                }

                caption->captionTextBookmark = captionTextBookmark;
                caption->labelNumBookmark = labelNumBookmark;
                caption->textBookmark = textBookmark;

                break;
            }
        }
    }
    free(sortedIds);
    DFHashTableRelease(referencesById);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         WordRawBookmark                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordRawBookmark WordRawBookmark;

struct WordRawBookmark {
    DFNode *startElement;
    DFNode *endElement;
    int startOffset;
    int endOffset;
};

WordRawBookmark *WordRawBookmarkNew(void)
{
    WordRawBookmark *bookmark = (WordRawBookmark *)calloc(1,sizeof(WordRawBookmark));
    bookmark->startOffset = -1;
    bookmark->endOffset = -1;
    return bookmark;
}

void WordRawBookmarkFree(WordRawBookmark *bookmark)
{
    free(bookmark);
}

int WordRawBookmarkSize(WordRawBookmark *bookmark)
{
    if ((bookmark->startOffset >= 0) && (bookmark->endOffset >= 0) && (bookmark->endOffset >= bookmark->startOffset))
        return bookmark->endOffset - bookmark->startOffset;
    else
        return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       Bookmark collapsing                                      //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void findBookmarkSizes(DFNode *node, DFHashTable *bookmarksById, int *offset)
{
    switch (node->tag) {
        case WORD_BOOKMARKSTART:
        case WORD_BOOKMARKEND: {
            const char *bookmarkId = DFGetAttribute(node,WORD_ID);
            if (bookmarkId == NULL)
                bookmarkId = "";;
            WordRawBookmark *bookmark = DFHashTableLookup(bookmarksById,bookmarkId);
            if (bookmark == NULL) {
                bookmark = WordRawBookmarkNew();
                DFHashTableAdd(bookmarksById,bookmarkId,bookmark);
            }
            if (node->tag == WORD_BOOKMARKSTART) {
                bookmark->startElement = node;
                bookmark->startOffset = *offset;
            }
            else {
                bookmark->endElement = node;
                bookmark->endOffset = *offset;
            }
            break;
        }
        default:
            (*offset)++;
            break;
    }

    for (DFNode *child = node->first; child != NULL; child = child->next) {
        findBookmarkSizes(child,bookmarksById,offset);
    }
}

static int compareStartElements(void *thunk, const void *obj1, const void *obj2)
{
    DFHashTable *bookmarksById = (DFHashTable *)thunk;
    DFNode *element1 = *(DFNode **)obj1;
    DFNode *element2 = *(DFNode **)obj2;
    const char *id1 = DFGetAttribute(element1,WORD_ID);
    const char *id2 = DFGetAttribute(element2,WORD_ID);
    if (id1 == NULL)
        id1 = "";
    if (id2 == NULL)
        id2 = "";;
    WordRawBookmark *bookmark1 = DFHashTableLookup(bookmarksById,id1);
    WordRawBookmark *bookmark2 = DFHashTableLookup(bookmarksById,id2);
    int size1 = WordRawBookmarkSize(bookmark1);
    int size2 = WordRawBookmarkSize(bookmark2);

    if (size1 > size2)
        return -1;
    else if (size1 < size2)
        return 1;
    else
        return DFStringCompare(id1,id2);
}

static void collapseRecursive(DFNode *node, DFHashTable *bookmarksById)
{

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;

        switch (child->tag) {
            case WORD_BOOKMARKSTART:
            case WORD_BOOKMARKEND: {
                DFArray *startElements = DFArrayNew(NULL,NULL);
                DFArray *endElements = DFArrayNew(NULL,NULL);
                DFHashTable *startIds = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
                DFHashTable *endIds = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
                DFNode *n;
                for (n = child;
                     (n != NULL) && ((n->tag == WORD_BOOKMARKSTART) || (n->tag == WORD_BOOKMARKEND));
                     n = n->next) {

                    if (n->tag == WORD_BOOKMARKSTART) {
                        const char *idValue = DFGetAttribute(n,WORD_ID);
                        if (idValue == NULL)
                            idValue = "";

                        DFHashTableAdd(startIds,idValue,idValue);
                        DFArrayAppend(startElements,n);
                    }
                    else {
                        const char *idValue = DFGetAttribute(n,WORD_ID);
                        if (idValue == NULL)
                            idValue = "";

                        DFHashTableAdd(endIds,idValue,idValue);
                        DFArrayAppend(endElements,n);
                    }
                }
                next = n;

                DFArraySort(startElements,bookmarksById,compareStartElements);

                for (size_t endIndex = 0; endIndex < DFArrayCount(endElements); endIndex++) {
                    DFNode *elem = DFArrayItemAt(endElements,endIndex);
                    const char *endId = DFGetAttribute(elem,WORD_ID);
                    int found = 0;
                    DFNode *ancestor;
                    for (ancestor = elem->parent; (ancestor != NULL) && !found; ancestor = ancestor->parent) {
                        if ((ancestor->tag == WORD_BOOKMARK) && DFStringEquals(DFGetAttribute(ancestor,WORD_ID),endId)) {
                            found = 1;
                            break;
                        }
                    }

                    if (found) {
                        DFNode *before = ancestor->next;
                        DFNode *nnext;
                        for (DFNode *n = child; n != NULL; n = nnext) {
                            nnext = n->next;
                            DFInsertBefore(ancestor->parent,n,before);
                        }
                    }
                }

                size_t x = 0;
                while (x < DFArrayCount(startElements)) {
                    DFNode *element = DFArrayItemAt(startElements,x);
                    const char *bookmarkId = DFGetAttribute(element,WORD_ID);
                    if (bookmarkId == NULL)
                        bookmarkId = "";
                    if (DFHashTableLookup(endIds,bookmarkId) != NULL) {
                        element->tag = WORD_BOOKMARK;
                        DFArrayRemove(startElements,x);
                    }
                    else {
                        x++;
                    }
                }

                if (DFArrayCount(startElements) > 0) {
                    for (size_t i = 1; i < DFArrayCount(startElements); i++) {
                        DFNode *tempParent = DFArrayItemAt(startElements,i-1);
                        DFNode *tempChild = DFArrayItemAt(startElements,i);
                        DFAppendChild(tempParent,tempChild);
                    }

                    DFNode *last = DFArrayItemAt(startElements,DFArrayCount(startElements)-1);
                    while (next != NULL) {
                        DFNode *tempChild = next;
                        next = next->next;
                        DFAppendChild(last,tempChild);
                    }
                }

                for (size_t eIndex = 0; eIndex < DFArrayCount(startElements); eIndex++) {
                    DFNode *e = DFArrayItemAt(startElements,eIndex);
                    e->tag = WORD_BOOKMARK;
                }
                for (size_t eIndex = 0; eIndex < DFArrayCount(endElements); eIndex++) {
                    DFNode *e = DFArrayItemAt(endElements,eIndex);
                    DFRemoveNode(e);
                }

                if (DFArrayCount(startElements) > 0) {
                    DFNode *last = DFArrayItemAt(startElements,DFArrayCount(startElements)-1);
                    collapseRecursive(last,bookmarksById);
                }

                DFArrayRelease(startElements);
                DFArrayRelease(endElements);
                DFHashTableRelease(startIds);
                DFHashTableRelease(endIds);
                break;
            }
            default:
                collapseRecursive(child,bookmarksById);
                break;
        }
    }
}

static DFNode *findParagraphBackwards(DFNode *node)
{
    do {
        node = DFPrevNode(node);
    }
    while ((node != NULL) && (node->tag != WORD_P));
    return node;
}

static DFNode *findParagraphForwards(DFNode *node)
{
    do {
        node = DFNextNode(node);
    }
    while ((node != NULL) && (node->tag != WORD_P));
    return node;
}

static void putInParagraphsRecursive(DFNode *node)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        putInParagraphsRecursive(child);
    }

    if (((node->tag == WORD_BOOKMARKSTART) || (node->tag == WORD_BOOKMARKEND)) &&
        (node->parent->tag != WORD_P)) {
        DFNode *forwards = findParagraphForwards(node);
        if (forwards != NULL) {
            DFNode *pPr = DFChildWithTag(forwards,WORD_PPR);
            if (pPr != NULL)
                DFInsertBefore(forwards,node,pPr->next);
            else
                DFInsertBefore(forwards,node,forwards->first);
            return;
        }

        DFNode *backwards = findParagraphBackwards(node);
        if (backwards != NULL) {
            DFAppendChild(backwards,node);
            return;
        }

        DFRemoveNode(node);
    }
}

void WordBookmarks_collapseNew(DFDocument *doc)
{
    putInParagraphsRecursive(doc->docNode);
    DFHashTable *bookmarksById = DFHashTableNew(NULL,(DFFreeFunction)WordRawBookmarkFree);
    int offset = 0;
    findBookmarkSizes(doc->root,bookmarksById,&offset);
    collapseRecursive(doc->root,bookmarksById);
    DFHashTableRelease(bookmarksById);
}

static void expandRecursive(DFNode *node)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        expandRecursive(child);
    }

    if (node->tag == WORD_BOOKMARK) {
        const char *bookmarkId = DFGetAttribute(node,WORD_ID);
        const char *bookmarkName = DFGetAttribute(node,WORD_NAME);
        if (bookmarkId == NULL)
            bookmarkId = "";
        if (bookmarkName == NULL)
            bookmarkName = "";;
        DFNode *startElem = DFCreateElement(node->doc,WORD_BOOKMARKSTART);
        DFNode *endElem = DFCreateElement(node->doc,WORD_BOOKMARKEND);
        DFSetAttribute(startElem,WORD_ID,bookmarkId);
        DFSetAttribute(startElem,WORD_NAME,bookmarkName);
        DFSetAttribute(endElem,WORD_ID,bookmarkId);
        DFInsertBefore(node->parent,startElem,node);
        DFInsertBefore(node->parent,endElem,node->next);
        DFRemoveNodeButKeepChildren(node);
    }
}

void WordBookmarks_expandNew(DFDocument *doc)
{
    expandRecursive(doc->docNode);
}

static void removeCaptionBookmarksRecursive(DFNode *node, int inCaption)
{
    if (node->tag == WORD_P) {
        DFNode *pPr = DFChildWithTag(node,WORD_PPR);
        const char *styleId = DFGetChildAttribute(pPr,WORD_PSTYLE,WORD_VAL);
        if (DFStringEquals(styleId,"Caption"))
            inCaption = 1;
    }

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        removeCaptionBookmarksRecursive(child,inCaption);
    }

    if (inCaption) {
        switch (node->tag) {
            case WORD_BOOKMARKSTART:
            case WORD_BOOKMARKEND:
            case WORD_BOOKMARK:
                DFRemoveNodeButKeepChildren(node);
                break;
        }
    }
}

void WordBookmarks_removeCaptionBookmarks(DFDocument *doc)
{
    removeCaptionBookmarksRecursive(doc->docNode,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        WordBookmarkLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int WordBookmarkIsVisible2(DFNode *concrete);

static DFNode *WordBookmarkGet(WordGetData *get, DFNode *concrete)
{
    if (!WordBookmarkIsVisible2(concrete))
        return NULL;;
    DFNode *abstract = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
    DFSetAttribute(abstract,HTML_CLASS,DFBookmarkClass);
    WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);
    return abstract;
}

static int WordBookmarkIsVisible2(DFNode *concrete)
{
    const char *name = DFGetAttribute(concrete,WORD_NAME);

    if (name == NULL)
        return 0;

    if (DFStringEquals(name,"_GoBack") && (concrete->first == NULL))
        return 0;

    return 1;
}

static int WordBookmarkIsVisible(WordPutData *put, DFNode *concrete)
{
    return WordBookmarkIsVisible2(concrete);
}

static void WordBookmarkPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);
}

static void WordBookmarkRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordParagraphContentLens.remove(put,child);
}

WordLens WordBookmarkLens = {
    .isVisible = WordBookmarkIsVisible,
    .get = WordBookmarkGet,
    .put = WordBookmarkPut,
    .create = NULL, // LENS FIXME
    .remove = WordBookmarkRemove,
};
