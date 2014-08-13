//
//  WordBody.c
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
//                                          WordBodylens                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordBodyGet(WordGetData *get, DFNode *concrete)
{
    DFNode *abstract = WordConverterCreateAbstract(get,HTML_BODY,concrete);
    WordContainerGet(get,&WordBlockLevelLens,abstract,concrete);
    return abstract;
}

static void WordBodyPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordBlockLevelLens,abstract,concrete);
}

static void WordBodyRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordBlockLevelLens.remove(put,child);
}

WordLens WordBodyLens = {
    .isVisible = NULL, // LENS FIXME
    .get = WordBodyGet,
    .put = WordBodyPut,
    .create = NULL, // LENS FIXME
    .remove = WordBodyRemove,
};
