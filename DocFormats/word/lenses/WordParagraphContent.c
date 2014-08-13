//
//  WordParagraphContent.c
//  DocFormats
//
//  Created by Peter Kelly on 2/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordLenses.h"
#include "WordBookmark.h"
#include "WordObjects.h"
#include "DFDOM.h"
#include "DFString.h"
#include "DFHTML.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                    WordParagraphContentLens                                    //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static WordLens *WordParagraphContentLensForConcrete(WordConverter *converter, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_R:
            return &WordRunLens;
        case WORD_INS:
        case WORD_DEL:
        case WORD_MOVEFROM:
        case WORD_MOVETO:
            return &WordChangeLens;
        case WORD_FLDSIMPLE:
            return &WordFieldLens;
        case WORD_BOOKMARK:
            return &WordBookmarkLens;
        case WORD_HYPERLINK:
            return &WordHyperlinkLens;
        case WORD_SMARTTAG:
            return &WordSmartTagLens;
        default:
            return NULL;
    }
}

static DFNode *WordParagraphContentGet(WordGetData *get, DFNode *concrete)
{
    WordLens *lens = WordParagraphContentLensForConcrete(get->conv,concrete);
    if ((lens != NULL) && (lens->get != NULL))
        return lens->get(get,concrete);
    else
        return NULL;
}

static int WordParagraphContentIsVisible(WordPutData *put, DFNode *concrete)
{
    WordLens *lens = WordParagraphContentLensForConcrete(put->conv,concrete);
    if ((lens != NULL) && (lens->isVisible != NULL))
        return lens->isVisible(put,concrete);
    else
        return 0;
}

static void WordParagraphContentPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordLens *lens = WordParagraphContentLensForConcrete(put->conv,concrete);
    if ((lens != NULL) && (lens->put != NULL))
        lens->put(put,abstract,concrete);
}

static void WordParagraphContentRemove(WordPutData *put, DFNode *concrete)
{
    WordLens *lens = WordParagraphContentLensForConcrete(put->conv,concrete);
    if ((lens != NULL) && (lens->remove != NULL))
        lens->remove(put,concrete);
}

static DFNode *WordParagraphContentCreate(WordPutData *put, DFNode *abstract)
{
    switch (abstract->tag) {
        case HTML_SPAN: {
            const char *spanClass = DFGetAttribute(abstract,HTML_CLASS);
            if (DFStringEquals(spanClass,DFFieldClass)) {
                DFNode *concrete = DFCreateElement(put->conv->package->document,WORD_FLDSIMPLE);
                char *nodeText = DFNodeTextToString(abstract);
                DFSetAttribute(concrete,WORD_INSTR,nodeText);
                free(nodeText);
                put->conv->haveFields = 1;
                return concrete;
            }
            else if (DFStringEquals(spanClass,DFBookmarkClass)) {
                const char *bookmarkId = DFGetAttribute(abstract,WORD_ID);
                const char *bookmarkName = DFGetAttribute(abstract,WORD_NAME);
                if ((bookmarkId == NULL) || (bookmarkName == NULL))
                    return NULL;
                WordBookmark *bookmark = WordObjectsBookmarkWithName(put->conv->objects,bookmarkName);
                if (bookmark == NULL)
                    return NULL;
                DFNode *concrete = DFCreateElement(put->conv->package->document,WORD_BOOKMARK);
                DFSetAttribute(concrete,WORD_ID,bookmarkId);
                DFSetAttribute(concrete,WORD_NAME,bookmarkName);
                bookmark->element = concrete;
                WordParagraphContentPut(put,abstract,concrete);
                return concrete;
            }
            else {
                DFNode *concrete = DFCreateElement(put->conv->package->document,WORD_R);
                WordParagraphContentPut(put,abstract,concrete);
                return concrete;
            }
        }
        case HTML_INS:
        case HTML_DEL:
            return WordChangeLens.create(put,abstract);
        case HTML_A: {
            const char *href = DFGetAttribute(abstract,HTML_HREF);
            if (href != NULL) {
                if (HTML_nodeIsHyperlink(abstract))
                    return WordHyperlinkLens.create(put,abstract);
                else
                    return WordFieldLens.create(put,abstract);
            }
            return NULL;
        }
        default:
            return NULL;
    }
}

WordLens WordParagraphContentLens = {
    .isVisible = WordParagraphContentIsVisible,
    .get = WordParagraphContentGet,
    .put = WordParagraphContentPut,
    .create = WordParagraphContentCreate,
    .remove = WordParagraphContentRemove,
};
