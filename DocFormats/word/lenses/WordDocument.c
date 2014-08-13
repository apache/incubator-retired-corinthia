//
//  WordDocument.c
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
//                                        WordDocumentLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordDocumentGet(WordGetData *get, DFNode *concrete)
{
    if (concrete->tag != WORD_DOCUMENT)
        return NULL;

    DFNode *html = WordConverterCreateAbstract(get,HTML_HTML,concrete);
    DFNode *head = WordConverterCreateAbstract(get,HTML_HEAD,NULL);
    DFAppendChild(html,head);
    DFNode *meta = WordConverterCreateAbstract(get,HTML_META,NULL);
    DFAppendChild(head,meta);
    DFSetAttribute(meta,HTML_CHARSET,"utf-8");

    DFNode *wordBody = DFChildWithTag(concrete,WORD_BODY);
    if (wordBody != NULL) {
        DFNode *htmlBody = WordBodyLens.get(get,wordBody);
        DFAppendChild(html,htmlBody);
    }
    return html;
}

static void WordDocumentPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    if ((abstract->tag == HTML_HTML) && (concrete->tag == WORD_DOCUMENT)) {
        DFNode *htmlBody = DFChildWithTag(abstract,HTML_BODY);
        DFNode *wordBody = DFChildWithTag(concrete,WORD_BODY);

        if ((htmlBody != NULL) && (wordBody != NULL))
            WordBodyLens.put(put,htmlBody,wordBody);
    }
}

WordLens WordDocumentLens = {
    .isVisible = NULL, // LENS FIXME
    .get = WordDocumentGet,
    .put = WordDocumentPut,
    .create = NULL, // LENS FIXME
    .remove = NULL, // LENS FIXME
};
