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

#include "DFPlatform.h"
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
        return NULL;;

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
