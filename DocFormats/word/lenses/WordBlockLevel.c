//
//  WordBlockLevel.c
//  DocFormats
//
//  Created by Peter Kelly on 2/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordLenses.h"
#include "DFDOM.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       WordBlockLevelLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordBlockLevelGet(WordGetData *get, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            return WordParagraphLens.get(get,concrete);
        case WORD_TBL:
            return WordTableLens.get(get,concrete);
        default:
            return NULL;
    }
}

static int WordBlockLevelIsVisible(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            return WordParagraphLens.isVisible(put,concrete);
        case WORD_TBL:
            return WordTableLens.isVisible(put,concrete);
        default:
            return 0;
    }
}

static void WordBlockLevelPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            WordParagraphLens.put(put,abstract,concrete);
            break;
        case WORD_TBL:
            WordTableLens.put(put,abstract,concrete);
            break;
        default:
            break;
    }
}

static void WordBlockLevelRemove(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            WordParagraphLens.remove(put,concrete);
            break;
        case WORD_TBL:
            WordTableLens.remove(put,concrete);
            break;
        default:
            break;
    }
}

static DFNode *WordBlockLevelCreate(WordPutData *put, DFNode *abstract)
{
    DFNode *concrete = NULL;
    switch (abstract->tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
        case HTML_FIGURE:
            return WordParagraphLens.create(put,abstract);
        case HTML_TABLE:
            concrete = DFCreateElement(put->conv->package->document,WORD_TBL);
            break;
    }
    if (concrete != NULL)
        WordBlockLevelPut(put,abstract,concrete);
    return concrete;
}

WordLens WordBlockLevelLens = {
    .isVisible = WordBlockLevelIsVisible,
    .get = WordBlockLevelGet,
    .put = WordBlockLevelPut,
    .create = WordBlockLevelCreate,
    .remove = WordBlockLevelRemove,
};
