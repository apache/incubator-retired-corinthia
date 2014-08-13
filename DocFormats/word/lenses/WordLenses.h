//
//  WordLenses.h
//  DocFormats
//
//  Created by Peter Kelly on 2/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordLenses_h
#define DocFormats_WordLenses_h

#include "DFBDT.h"
#include "WordConverter.h"

typedef struct {
    int (*isVisible)(WordPutData *put, DFNode *concrete);
    DFNode *(*get)(WordGetData *get, DFNode *concrete);
    void (*put)(WordPutData *put, DFNode *abstract, DFNode *concrete);
    DFNode *(*create)(WordPutData *put, DFNode *abstract);
    void (*remove)(WordPutData *put, DFNode *concrete);
} WordLens;

extern WordLens WordBlockLevelLens;
extern WordLens WordBodyLens;
extern WordLens WordBookmarkLens;
extern WordLens WordChangeLens;
extern WordLens WordDocumentLens;
extern WordLens WordDrawingLens;
extern WordLens WordEquationLens;
extern WordLens WordFieldLens;
extern WordLens WordHyperlinkLens;
extern WordLens WordParagraphLens;
extern WordLens WordParagraphContentLens;
extern WordLens WordRunLens;
extern WordLens WordRunContentLens;
extern WordLens WordSmartTagLens;
extern WordLens WordTableLens;

DFNode *WordContainerGet(WordGetData *get, WordLens *childLens, DFNode *abstract, DFNode *concrete);
void WordContainerPut(WordPutData *put, WordLens *childLens, DFNode *abstract, DFNode *concrete);

#endif
