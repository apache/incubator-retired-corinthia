//
//  WordChange.c
//  DocFormats
//
//  Created by Peter Kelly on 14/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordLenses.h"
#include "DFDOM.h"
#include "DFString.h"
#include "DFCommon.h"

static void WordChangePut(WordPutData *put, DFNode *abstract, DFNode *concrete);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         WordChangeLens                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordChangeGet(WordGetData *get, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_INS: {
            DFNode *abstract = WordConverterCreateAbstract(get,HTML_INS,concrete);
            WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);
            return abstract;
        }
        case WORD_DEL: {
            DFNode *abstract = WordConverterCreateAbstract(get,HTML_DEL,concrete);
            WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);
            return abstract;
        }
//        case WORD_MOVEFROM:
//        case WORD_MOVETO:
//            break;
        default:
            return NULL;
    }
}

static int WordChangeIsVisible(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_INS:
        case WORD_DEL:
            return 1;
        default:
            return 0;
    }
}

static DFNode *WordChangeCreate(WordPutData *put, DFNode *abstract)
{
    switch (abstract->tag) {
        case HTML_INS: {
            DFNode *concrete = DFCreateElement(put->conv->package->document,WORD_INS);
            WordChangePut(put,abstract,concrete);
            return concrete;
        }
        case HTML_DEL: {
            DFNode *concrete = DFCreateElement(put->conv->package->document,WORD_INS);
            WordChangePut(put,abstract,concrete);
            return concrete;
        }
        default:
            return NULL;
    }
}

static void WordChangePut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);
}

static void WordChangeRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordParagraphContentLens.remove(put,child);
}

WordLens WordChangeLens = {
    .isVisible = WordChangeIsVisible,
    .get = WordChangeGet,
    .put = WordChangePut,
    .create = WordChangeCreate,
    .remove = WordChangeRemove,
};
