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
#include "ODFLenses.h"
#include "DFDOM.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        ODFDocumentLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The general pattern for a lens is
 *      for the type of tag create the corresponding abstract DFNode
 *      via call to ODFConverterCreateAbstract
 *      eg TEXT_P -> HTML_P
 *      then add attributes as required
 *
 *      call ODFContainerGet to walk down through the concrete tree
 *      to build on the abstact DFNode
 *
 *      return the abstract DFNode
 */


/**
 * Not a standard lens because we manually find the OFFICE_BODY
 * and call its lens and append its node to the html node
 */
static DFNode *ODFDocumentGet(ODFGetData *get, DFNode *concrete)
{
    printf("ODFDocumentGet\n");
    if (concrete->tag != OFFICE_DOCUMENT_CONTENT)
        return NULL;;

    DFNode *html = ODFConverterCreateAbstract(get,HTML_HTML,concrete);
    DFNode *head = ODFConverterCreateAbstract(get,HTML_HEAD,NULL);
    DFAppendChild(html,head);
    DFNode *meta = ODFConverterCreateAbstract(get,HTML_META,NULL);
    DFAppendChild(head,meta);
    DFSetAttribute(meta,HTML_CHARSET,"utf-8");

    //Find the OFFICE_BODY
    DFNode *ODFBody = DFChildWithTag(concrete,OFFICE_BODY);
    if (ODFBody != NULL) {
        DFNode *htmlBody = ODFBodyLens.get(get,ODFBody);
        DFAppendChild(html,htmlBody);
    }
    return html;
}

static void ODFDocumentPut(ODFPutData *put, DFNode *abstract, DFNode *concrete)
{
    if ((abstract->tag == HTML_HTML) && (concrete->tag == OFFICE_DOCUMENT_CONTENT)) {
        DFNode *htmlBody = DFChildWithTag(abstract,HTML_BODY);
        DFNode *ODFBody = DFChildWithTag(concrete,OFFICE_BODY);

        if ((htmlBody != NULL) && (ODFBody != NULL))
            ODFBodyLens.put(put,htmlBody,ODFBody);
    }
}

ODFLens ODFDocumentLens = {
    .isVisible = NULL, // LENS FIXME
    .get = ODFDocumentGet,
    .put = ODFDocumentPut,
    .create = NULL, // LENS FIXME
    .remove = NULL, // LENS FIXME
};
