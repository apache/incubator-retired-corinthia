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

#include "WordObjects.h"
#include "WordPackage.h"
#include "WordDrawing.h"
#include "WordBookmark.h"
#include "WordCaption.h"
#include "DFDOM.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordObjects                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordObjects {
    WordPackage *package;
    DFHashTable *drawingsById;
    DFHashTable *bookmarksById;
    DFHashTable *bookmarksByName;
    DFHashTable *captionsByTarget;
    int nextDrawingId;
    int nextBookmarkNameNum;
    int nextBookmarkId;
};

WordObjects *WordObjectsNew(WordPackage *package)
{
    WordObjects *objects = (WordObjects *)calloc(1,sizeof(WordObjects));
    objects->package = WordPackageRetain(package);
    objects->drawingsById = DFHashTableNew((DFCopyFunction)WordDrawingRetain,(DFFreeFunction)WordDrawingRelease);
    objects->bookmarksById = DFHashTableNew((DFCopyFunction)WordBookmarkRetain,(DFFreeFunction)WordBookmarkRelease);
    objects->bookmarksByName = DFHashTableNew((DFCopyFunction)WordBookmarkRetain,(DFFreeFunction)WordBookmarkRelease);
    objects->captionsByTarget = DFHashTableNew((DFCopyFunction)WordCaptionRetain,(DFFreeFunction)WordCaptionRelease);
    objects->nextDrawingId = 1;
    objects->nextBookmarkNameNum = 0;
    objects->nextBookmarkId = 0;
    return objects;
}

void WordObjectsFree(WordObjects *objects)
{
    DFHashTableRelease(objects->drawingsById);
    DFHashTableRelease(objects->bookmarksById);
    DFHashTableRelease(objects->bookmarksByName);
    DFHashTableRelease(objects->captionsByTarget);
    WordPackageRelease(objects->package);
    free(objects);
}

static void scanRecursive(WordObjects *objects, DFNode *node)
{
    switch (node->tag) {
        case WORD_BOOKMARK: {
            const char *bookmarkId = DFGetAttribute(node,WORD_ID);
            const char *bookmarkName = DFGetAttribute(node,WORD_NAME);
            if ((bookmarkId == NULL) ||
                (bookmarkName == NULL) ||
                (WordObjectsBookmarkWithId(objects,bookmarkId) != NULL) ||
                (WordObjectsBookmarkWithName(objects,bookmarkName) != NULL)) {
                DFRemoveNode(node);
                return;
            }
            WordBookmark *bookmark = WordObjectsAddBookmarkWithId(objects,bookmarkId,bookmarkName);
            bookmark->element = node;
            break;
        }
        case DML_WP_DOCPR: {
            const char *drawingId = DFGetAttribute(node,NULL_ID);
            if ((drawingId != NULL) && (WordObjectsDrawingWithId(objects,drawingId) == NULL))
                WordObjectsAddDrawingWithId(objects,drawingId);
            break;
        }
    }

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        scanRecursive(objects,child);
    }
}

void WordObjectsScan(WordObjects *objects)
{
    scanRecursive(objects,objects->package->document->docNode);
}

WordDrawing *WordObjectsDrawingWithId(WordObjects *objects, const char *drawingId)
{
    return DFHashTableLookup(objects->drawingsById,drawingId);
}

WordDrawing *WordObjectsAddDrawingWithId(WordObjects *objects, const char *drawingId)
{
    WordDrawing *drawing = WordDrawingNew(drawingId);
    DFHashTableAdd(objects->drawingsById,drawingId,drawing);
    WordDrawingRelease(drawing);
    return drawing;
}

static char *createDrawingId(WordObjects *objects)
{
    while (1) {
        char *idStr = DFFormatString("%d",objects->nextDrawingId);
        if (DFHashTableLookup(objects->drawingsById,idStr) == NULL)
            return idStr;
        objects->nextDrawingId++;
        free(idStr);
    }
}

WordDrawing *WordObjectsAddDrawing(WordObjects *objects)
{
    char *drawingId = createDrawingId(objects);
    WordDrawing *drawing = WordObjectsAddDrawingWithId(objects,drawingId);
    free(drawingId);
    return drawing;
}

struct WordBookmark *WordObjectsBookmarkWithId(WordObjects *objects, const char *bookmarkId)
{
    return DFHashTableLookup(objects->bookmarksById,bookmarkId);
}

struct WordBookmark *WordObjectsBookmarkWithName(WordObjects *objects, const char *bookmarkName)
{
    return DFHashTableLookup(objects->bookmarksByName,bookmarkName);
}

struct WordBookmark *WordObjectsAddBookmarkWithId(WordObjects *objects, const char *bookmarkId, const char *bookmarkName)
{
    WordBookmark *bookmark = WordBookmarkNew(bookmarkId,bookmarkName);
    DFHashTableAdd(objects->bookmarksById,bookmarkId,bookmark);
    DFHashTableAdd(objects->bookmarksByName,bookmarkName,bookmark);
    WordBookmarkRelease(bookmark);
    return bookmark;
}

static char *createBookmarkName(WordObjects *objects)
{
    while (1) {
        char *nameStr = DFFormatString("uxwrite%d",objects->nextBookmarkNameNum);
        if (DFHashTableLookup(objects->bookmarksByName,nameStr) == NULL)
            return nameStr;
        objects->nextBookmarkNameNum++;
        free(nameStr);
    }
}

static char *createBookmarkId(WordObjects *objects)
{
    while (1) {
        char *idStr = DFFormatString("%d",objects->nextBookmarkId);
        if (DFHashTableLookup(objects->bookmarksById,idStr) == NULL)
            return idStr;
        objects->nextBookmarkId++;
        free(idStr);
    }
}

struct WordBookmark *WordObjectsAddBookmark(WordObjects *objects)
{
    char *bookmarkId = createBookmarkId(objects);
    char *bookmarkName = createBookmarkName(objects);
    WordBookmark *bookmark = WordObjectsAddBookmarkWithId(objects,bookmarkId,bookmarkName);
    free(bookmarkId);
    free(bookmarkName);
    return bookmark;
}

void WordObjectsCollapseBookmarks(WordObjects *objects)
{
    WordBookmarks_collapseNew(objects->package->document);
}

void WordObjectsAnalyzeBookmarks(WordObjects *objects, WordSheet *sheet)
{
    const char **keys = DFHashTableCopyKeys(objects->bookmarksById);
    for (int i = 0; keys[i]; i++) {
        WordBookmark *bookmark = DFHashTableLookup(objects->bookmarksById,keys[i]);
        WordBookmarkAnalyze(bookmark,sheet);
    }
    free(keys);
}

void WordObjectsExpandBookmarks(WordObjects *objects)
{
    WordBookmarks_expandNew(objects->package->document);
}

struct WordCaption *WordObjectsCaptionForTarget(WordObjects *objects, DFNode *target)
{
    assert((target->tag == HTML_TABLE) || (target->tag == HTML_FIGURE));
    return DFHashTableLookupInt(objects->captionsByTarget,target->seqNo);
}

void WordObjectsSetCaption(WordObjects *objects, struct WordCaption *caption, DFNode *target)
{
    assert((target->tag == HTML_TABLE) || (target->tag == HTML_FIGURE));
    DFHashTableAddInt(objects->captionsByTarget,target->seqNo,caption);
}
