//
//  WordBookmark.h
//  DocFormats
//
//  Created by Peter Kelly on 4/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordBookmark_h
#define DocFormats_WordBookmark_h

#include "DFXMLForward.h"
#include "WordSheet.h"

struct WordPutData;

void Word_setupBookmarkLinks(struct WordPutData *put);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordBookmark                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    WordBookmarkUnknown,
    WordBookmarkCursor,
    WordBookmarkHeading,
    WordBookmarkTable,
    WordBookmarkFigure,
    WordBookmarkEquation,
} WordBookmarkType;

typedef struct {
    int beforeNum;
    int num;
    int afterNum;
} CaptionParts;

const char *WordBookmarkTypeString(WordBookmarkType type);
int WordBookmarkTypeHasLabel(WordBookmarkType type);

typedef struct WordBookmark WordBookmark;

struct WordBookmark {
    size_t retainCount;
    char *bookmarkId;
    char *bookmarkName;
    DFNode *element;
    WordBookmarkType type;
    char *label;
    DFNode *target;
};

WordBookmark *WordBookmarkNew(const char *bookmarkId, const char *bookmarkName);
WordBookmark *WordBookmarkRetain(WordBookmark *bookmark);
void WordBookmarkRelease(WordBookmark *bookmark);
void WordBookmarkAnalyze(WordBookmark *bookmark, WordSheet *sheet);
CaptionParts WordBookmarkGetCaptionParts(WordBookmark *bookmark);

DFNode *WordFindContainingParagraph(DFNode *node);

void WordBookmarks_collapseNew(DFDocument *doc);
void WordBookmarks_expandNew(DFDocument *doc);
void WordBookmarks_removeCaptionBookmarks(DFDocument *doc);

#endif
