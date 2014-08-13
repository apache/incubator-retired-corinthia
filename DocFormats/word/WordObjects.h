//
//  WordObjects.h
//  DocFormats
//
//  Created by Peter Kelly on 25/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordObjects_h
#define DocFormats_WordObjects_h

#include "DFXMLForward.h"
#include "WordSheet.h"
#include "WordPackage.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordObjects                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordObjects WordObjects;

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
