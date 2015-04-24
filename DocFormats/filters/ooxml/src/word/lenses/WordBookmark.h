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

#ifndef DocFormats_WordBookmark_h
#define DocFormats_WordBookmark_h

#include <DocFormats/DFXMLForward.h>
#include "WordSheet.h"
#include "OOXMLTypedefs.h"

struct WordPutData;

void Word_setupBookmarkLinks(struct WordPutData *put);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordBookmark                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

enum WordBookmarkType {
    WordBookmarkUnknown,
    WordBookmarkCursor,
    WordBookmarkHeading,
    WordBookmarkTable,
    WordBookmarkFigure,
    WordBookmarkEquation,
};

struct CaptionParts{
    int beforeNum;
    int num;
    int afterNum;
};

const char *WordBookmarkTypeString(WordBookmarkType type);
int WordBookmarkTypeHasLabel(WordBookmarkType type);

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
