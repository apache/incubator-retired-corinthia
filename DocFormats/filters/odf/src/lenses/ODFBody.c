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
#include "DFCommon.h"
#include "DFDOM.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          ODFBodylens                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Not a standard lens because we switch to the correct lense
 * from the first child
 */
static DFNode *ODFBodyGet(ODFGetData *get, DFNode *concrete)
{
    printf("ODFBodyGet\n");
    DFNode *abstract = ODFConverterCreateAbstract(get,HTML_BODY,concrete);
    //An office:body element contains the text,spreadsheet, or presenation
    //elements (plus others we are not going to consider.
    DFNode* docType = concrete->first;
    switch (docType->tag) {
        case OFFICE_TEXT:
            ODFContainerGet(get,&ODFTextLens,abstract,concrete);
            break;
        case OFFICE_SPREADSHEET:
            ODFContainerGet(get,&ODFSpreadsheetLens,abstract,concrete);
            break;
        case OFFICE_PRESENTATION:
            ODFContainerGet(get,&ODFPresentationLens,abstract,concrete);
            break;
        default:
            //Add a message to the html document to say document type not supported
            break;
    }

    return abstract;
}

static void ODFBodyPut(ODFPutData *put, DFNode *abstract, DFNode *concrete)
{
    //ODFContainerPut(put,&ODFBlockLevelLens,abstract,concrete);
}

static void ODFBodyRemove(ODFPutData *put, DFNode *concrete)
{
    //for (DFNode *child = concrete->first; child != NULL; child = child->next)
    //    ODFBlockLevelLens.remove(put,child);
}

ODFLens ODFBodyLens = {
    .isVisible = NULL, // LENS FIXME
    .get = ODFBodyGet,
    .put = ODFBodyPut,
    .create = NULL, // LENS FIXME
    .remove = ODFBodyRemove,
};
