//
//  WordSmartTag.c
//  DocFormats
//
//  Created by Peter Kelly on 7/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordLenses.h"
#include "DFDOM.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        WordSmartTagLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordSmartTagGet(WordGetData *get, DFNode *concrete)
{
    DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
    DFSetAttribute(span,HTML_CLASS,DFContainerClass);
    WordContainerGet(get,&WordParagraphContentLens,span,concrete);
    return span;
}

static int WordSmartTagIsVisible(WordPutData *put, DFNode *concrete)
{
    return 1;
}

static void WordSmartTagPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);

    // If there is a smartTagPr element, it must come first
    DFNode *smartTagPr = DFChildWithTag(concrete,WORD_SMARTTAGPR);
    if (smartTagPr != NULL)
        DFInsertBefore(concrete,smartTagPr,concrete->first);
}

static void WordSmartTagRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordParagraphContentLens.remove(put,child);
}

WordLens WordSmartTagLens = {
    .isVisible = WordSmartTagIsVisible,
    .get = WordSmartTagGet,
    .put = WordSmartTagPut,
    .create = NULL, // LENS FIXME
    .remove = WordSmartTagRemove,
};
