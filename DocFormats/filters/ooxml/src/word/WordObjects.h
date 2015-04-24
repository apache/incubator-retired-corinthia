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

#ifndef DocFormats_WordObjects_h
#define DocFormats_WordObjects_h

#include <DocFormats/DFXMLForward.h>
#include "WordSheet.h"
#include "WordPackage.h"
#include "OOXMLTypedefs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordObjects                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordObjects *WordObjectsNew(WordPackage *package);
void WordObjectsFree(WordObjects *objects);

void WordObjectsScan(WordObjects *objects);

struct WordDrawing *WordObjectsDrawingWithId(WordObjects *objects, const char *drawingId);
struct WordDrawing *WordObjectsAddDrawingWithId(WordObjects *objects, const char *drawingId);
struct WordDrawing *WordObjectsAddDrawing(WordObjects *objects);

struct WordBookmark *WordObjectsBookmarkWithId(WordObjects *objects, const char *bookmarkId);
struct WordBookmark *WordObjectsBookmarkWithName(WordObjects *objects, const char *bookmarkName);
struct WordBookmark *WordObjectsAddBookmarkWithId(WordObjects *objects, const char *bookmarkId, const char *bookmarkName);
struct WordBookmark *WordObjectsAddBookmark(WordObjects *objects);
void WordObjectsCollapseBookmarks(WordObjects *objects);
void WordObjectsAnalyzeBookmarks(WordObjects *objects, WordSheet *sheet);
void WordObjectsExpandBookmarks(WordObjects *objects);

struct WordCaption *WordObjectsCaptionForTarget(WordObjects *objects, DFNode *target);
void WordObjectsSetCaption(WordObjects *objects, struct WordCaption *caption, DFNode *target);

#endif
