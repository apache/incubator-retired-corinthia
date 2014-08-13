//
//  WordDrawing.h
//  DocFormats
//
//  Created by Peter Kelly on 25/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordDrawing_h
#define DocFormats_WordDrawing_h

#include "WordConverter.h"
#include "DFDOM.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordDrawing                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordDrawing WordDrawing;

struct WordDrawing {
    size_t retainCount;
    char *drawingId;
};

WordDrawing *WordDrawingNew(const char *drawingId);
WordDrawing *WordDrawingRetain(WordDrawing *drawing);
void WordDrawingRelease(WordDrawing *drawing);

DFNode *WordDrawingGet(WordGetData *get, DFNode *concrete);
int WordDrawingIsVisible(WordPutData *put, DFNode *concrete);
DFNode *WordDrawingCreate(WordPutData *put, DFNode *abstract);
void WordDrawingPut(WordPutData *put, DFNode *abstract, DFNode *concrete);
void WordDrawingRemove(WordPutData *put, DFNode *concrete);

#endif
